#include "OpcodeBase.hpp"
#include <pstream.h>
#include <algorithm>
#include <cstring>
#include <map>
#include <string>
#include <vector>
/**
 * T H E   S I G N A L   F L O W   G R A P H   O P C O D E S
 *
 * Michael Gogins
 *
 * These opcodes enable the use of signal flow graphs 
 * (AKA asynchronous data flow graphs) in Csound orchestras. 
 * Signals flow from the outlets of source instruments
 * and are summed in the inlets of sink instruments. 
 * Signals may be k-rate, a-rate, or f-rate.
 * Any number of outlets may be connected to any number of inlets. 
 * When a new instance of an instrument is instantiated during performance, 
 * the declared connections also are automatically instantiated.
 * 
 * Signal flow graphs simplify the construction of complex mixers, 
 * signal processing chains, and the like. They also simplify the re-use 
 * of "plug and play" instrument definitions and even entire sub-orchestras, 
 * which can simply be #included and then "plugged in" to existing orchestras.
 *
 * Note that inlets and outlets are defined in instruments without reference 
 * to how they are connected. Connections are defined in the orchestra header. 
 * It is this separation that enables plug-in instruments.
 *
 * Instruments must be named, and each source instrument must be defined 
 * in the orchestra before any of its sinks. The reason instruments must be 
 * named is so that outlets and inlets in any higher-level orchestra can
 * be connected to inlets and outlets in any lower-level #included orchestra. 
 *
 * O P C O D E S
 * 
 * signalflowgraph
 *
 * Initializes the signal flow graph; must be declared once and only once 
 * in the top-level orchestra, before any of the other 
 * signal flow graph opcodes.
 *
 * outleta Sname, asignal
 * outletk Sname, ksignal
 * outletf Sname, fsignal
 *
 * Outlets send a, k, or f-rate signals out from an instrument.
 *
 * The name of the outlet is implicitly qualified by the instrument name,
 * so it is valid to use the same outlet name in more than one instrument
 * (but not to use the same outlet name twice in the same instrument).
 *
 * asignal inleta Sname
 * ksignal inletk Sname
 * fsignal inletf Snameo
 *s
 * Inlets receive a, k, or f-rate signals from outlets in other instruments.
 * The signals from all the source outlet instances are summed 
 * in each sink inlet instance.
 *
 * The name of the inlet is implicitly qualified by the instrument name,
 * so it is valid to use the same inlet name in more than one instrument
 * (but not to use the same inlet name twice in the same instrument).
 *
 * connect Source1, Soutlet1, Sink1, Sinlet1
 *
 * The connect opcode, valid only in orchestra headers, sends the signals
 * from the indicated outlets in all instances of the indicated source
 * instrument to the indicated inlets in all instances of the indicated sink
 * instrument. 
 * 
 * alwayson Sinstrumentname [p4, ..., pn]
 *
 * Activates the indicated instrument in the orchestra header,
 * without need for an i statement. Instruments must be 
 * activated in the same order as they are defined.
 *
 * The alwayson opcode is designed to simplify 
 * the definition of re-usable orchestras with 
 * signal processing or effects chains and networks.
 *
 * When the instrument is activated, p1 is the insno, p2 is 0, and p3 is -1.
 * The optional pfields are sent to the instrument following p3.
 *
 * ifno ftgenonce ip1, ip2dummy, isize, igen, iarga, iargb [, ...]
 *
 * Enables the creation of function tables entirely inside 
 * instrument definitions, without any duplication of data.
 *
 * The ftgenonce opcode is designed to simplify writing instrument definitions 
 * that can be re-used in different orchestras simply by #including them
 * and plugging them into some output instrument. There is no need to define 
 * function tables either in the score, or in the orchestra header.
 *
 * The ftgenonce opcode is similar to ftgentmp, and has identical arguments.
 * However, function tables are neither duplicated nor deleted. Instead, 
 * all of the arguments to the opcode are concatenated to form the key to a 
 * lookup table that points to the function table number. Thus, every request 
 * to ftgenonce with the same arguments receives the same instance of the 
 * function table data. Every change in the value of any ftgenonce argument
 * causes the creation of a new function table.
 */

struct SignalFlowGraph;
struct Outleta;
struct Outletk;
struct Outletf;
struct Inleta;
struct Inletk;
struct Inletf;
struct Connect;
struct AlwaysOn;
struct FtGenOnce;

bool operator < (const EVTBLK &a, const EVTBLK &b)
{
  if (a.opcod < b.opcod) {
    return true;
  }
  size_t n = std::min(a.pcnt, b.pcnt);
  for (size_t i = 0; i < n; i++) {
    if (a.p[i] == SSTRCOD && b.p[i] == SSTRCOD) {
      int comparison = std::strcmp(a.strarg, b.strarg);
      if (comparison < 0) {
	return true;
      } else if (comparison > 0) {
	return false;
      }
    } else if (a.p[i] == SSTRCOD) {
      return true;
    } else if (b.p[i] == SSTRCOD) {
      return false;
    } else {
      if (a.p[i] < b.p[i]) {
	return true;
      }
    }
  }
  if (a.pcnt < b.pcnt) {
    return true;
  } else if (a.pcnt > b.pcnt) {
    return false;
  }
  // Equal...
  return false;
}
  
// Identifiers are always "sourcename:outletname" or "sinkname:inletname".

// TODO: For true thread-safety, access to shared data must be protected.

std::map<CSOUND *, std::map< std::string, std::vector< Outleta * > > > aoutletsForCsoundsForSourceOutletIds;
std::map<CSOUND *, std::map< std::string, std::vector< Outletk * > > > koutletsForCsoundsForSourceOutletIds;
std::map<CSOUND *, std::map< std::string, std::vector< Outletf * > > > foutletsForCsoundsForSourceOutletIds;
std::map<CSOUND *, std::map< std::string, std::vector< Inleta * > > > ainletsForCsoundsForSinkInletIds;
std::map<CSOUND *, std::map< std::string, std::vector< Inletk * > > > kinletsForCsoundsForSinkInletIds;
std::map<CSOUND *, std::map< std::string, std::vector< Inletf * > > > finletsForCsoundsForSinkInletIds;
std::map<CSOUND *, std::map< std::string, std::vector< std::string > > > connectionsForCsounds;
std::map<CSOUND *, std::map< EVTBLK, int > > functionTablesForCsoundsForEvtblks;
std::map<CSOUND *, std::vector< std::vector< std::vector<Outleta *> *> * > > aoutletVectorsForCsounds;
std::map<CSOUND *, std::vector< std::vector< std::vector<Outletk *> *> * > > koutletVectorsForCsounds;
std::map<CSOUND *, std::vector< std::vector< std::vector<Outletf *> *> * > > foutletVectorsForCsounds;
std::map<CSOUND *, std::vector<std::string> > stdStringsForCsounds;

/**
 * All it does is clear the data structures for the current instance of Csound,
 * in case they are full from a previous performance.
 */
struct SignalFlowGraph : public OpcodeBase<SignalFlowGraph>
{
  int init(CSOUND *csound)
  {
#pragma omp critical
    {
      aoutletsForCsoundsForSourceOutletIds[csound].clear();
      koutletsForCsoundsForSourceOutletIds[csound].clear();
      foutletsForCsoundsForSourceOutletIds[csound].clear();
      ainletsForCsoundsForSinkInletIds[csound].clear();
      kinletsForCsoundsForSinkInletIds[csound].clear();
      finletsForCsoundsForSinkInletIds[csound].clear();
      connectionsForCsounds[csound].clear();
      functionTablesForCsoundsForEvtblks[csound].clear();
    }
    return OK;
  };
};

struct Outleta : public OpcodeBase<Outleta>
{
  /**
   * Output.
   */
  MYFLT *asignal;
  /**
   * Input.
   */
  MYFLT *Sname;
  /**
   * State.
   */
  const char *sourceOutletId;
  int init(CSOUND *csound)
  {
    // May need to convert insno to name if not named.
    std::string sourceOutletId_ = ((INSTRTXT *)h.insdshead->nxti->optext)->insname;
    sourceOutletId_ += ":";
    sourceOutletId_ += csound->strarg2name(csound,
					   (char *) 0,
					   Sname,
					   (char *)"",
					   (int) csound->GetInputArgSMask(this));
    std::vector<Outleta *> &aoutlets = aoutletsForCsoundsForSourceOutletIds[csound][sourceOutletId_];
    if (std::find(aoutlets.begin(), aoutlets.end(), this) == aoutlets.end()) {
      aoutlets.push_back(this);
    }
    stdStringsForCsounds[csound].push_back(sourceOutletId_);
    sourceOutletId = stdStringsForCsounds[csound].back().c_str();
    return OK;
  }
};

struct Inleta : public OpcodeBase<Inleta>
{
  /**
   * Inputs.
   */
  MYFLT *Sname;
  MYFLT *asignal;
  /**
   * State.
   */
  const char *sinkInletId;
  std::vector< std::vector<Outleta *> *> *sourceOutlets;
  int ksmps;
  int init(CSOUND *csound)
  {
    ksmps = csound->GetKsmps(csound);
    if (std::find(aoutletVectorsForCsounds[csound].begin(), 
		  aoutletVectorsForCsounds[csound].end(), 
		  sourceOutlets) == aoutletVectorsForCsounds[csound].end()) {
      sourceOutlets = new std::vector< std::vector<Outleta *> *>;
      aoutletVectorsForCsounds[csound].push_back(sourceOutlets);
    }
    // May need to convert insno to name if not named.
    std::string sinkInletId_ = ((INSTRTXT *)h.insdshead->nxti->optext)->insname;
    sinkInletId_ += ":";
    sinkInletId_ += csound->strarg2name(csound,
					(char *)0,
					Sname,
					(char *)"",
					(int) csound->GetInputArgSMask(this));
    std::vector<Inleta *> &ainlets = ainletsForCsoundsForSinkInletIds[csound][sinkInletId_];
    if (std::find(ainlets.begin(), ainlets.end(), this) == ainlets.end()) {
      ainlets.push_back(this);
    }
    // Find source outlets connecting to this.
    // Any number of sources may connect to any number of sinks.
    std::vector<std::string> &sourceOutletIds = connectionsForCsounds[csound][sinkInletId_];
    for (size_t i = 0, n = sourceOutletIds.size(); i < n; i++) {
      const std::string &sourceOutletId = sourceOutletIds[i];
      std::vector<Outleta *> &aoutlets = aoutletsForCsoundsForSourceOutletIds[csound][sourceOutletId];
      if (std::find(sourceOutlets->begin(), sourceOutlets->end(), &aoutlets) != sourceOutlets->end()) {
	sourceOutlets->push_back(&aoutlets);
      }
    }
    stdStringsForCsounds[csound].push_back(sinkInletId_);
    sinkInletId = stdStringsForCsounds[csound].back().c_str();
    return OK;
  }
  /**
   * Sum arate values from active outlets feeding this inlet.
   */
  int audio(CSOUND *csound)
  {
    // Zero the inlet buffer.
    for (size_t signalI = 0; signalI < ksmps; signalI++) {
      asignal[signalI] = FL(0.0);
    }
    // Loop over the source connections...
    for (size_t sourceI = 0, sourceN = sourceOutlets->size(); 
	 sourceI < sourceN; 
	 sourceI++) {
      // Loop over the source connection instances...
      const std::vector<Outleta *> *instances = sourceOutlets->at(sourceI);
      for (size_t instanceI = 0, instanceN = instances->size(); 
	   instanceI < instanceN; 
	   instanceI++) {
	const Outleta *sourceOutlet = instances->at(instanceI);
	// Skip inactive instances.
	if (sourceOutlet->h.insdshead->actflg) {
	  // Loop over the samples in the inlet buffer.  
	  for (size_t signalI = 0; 
	       signalI < ksmps; 
	       signalI++) {
	    asignal[signalI] += sourceOutlet->asignal[signalI];
	  }
	}
      }
    }
    return OK;
  }
};

struct Outletk : public OpcodeBase<Outletk>
{
  /**
   * Output.
   */
  MYFLT *ksignal;
  /**
   * Input.
   */
  MYFLT *Sname;
  /**
   * State.
   */
  const char *sourceOutletId;
  int init(CSOUND *csound)
  {
    // May need to convert insno to name if not named.
    std::string sourceOutletId_ = ((INSTRTXT *)h.insdshead->nxti->optext)->insname;
    sourceOutletId_ += ":";
    sourceOutletId_ += csound->strarg2name(csound,
					   (char *) 0,
					   Sname,
					   (char *)"",
					   (int) csound->GetInputArgSMask(this));
    std::vector<Outletk *> &koutlets = koutletsForCsoundsForSourceOutletIds[csound][sourceOutletId_];
    if (std::find(koutlets.begin(), koutlets.end(), this) == koutlets.end()) {
      koutlets.push_back(this);
    }
    stdStringsForCsounds[csound].push_back(sourceOutletId_);
    sourceOutletId = stdStringsForCsounds[csound].back().c_str();
    return OK;
  }
};

struct Inletk : public OpcodeBase<Inletk>
{
  /**
   * Inputs.
   */
  MYFLT *Sname;
  MYFLT *ksignal;
  /**
   * State.
   */
  const char *sinkInletId;
  std::vector< std::vector<Outletk *> *> *sourceOutlets;
  int ksmps;
  int init(CSOUND *csound)
  {
    ksmps = csound->GetKsmps(csound);
    if (std::find(koutletVectorsForCsounds[csound].begin(), 
		  koutletVectorsForCsounds[csound].end(), 
		  sourceOutlets) == koutletVectorsForCsounds[csound].end()) {
      sourceOutlets = new std::vector< std::vector<Outletk *> *>;
      koutletVectorsForCsounds[csound].push_back(sourceOutlets);
    }
    // May need to convert insno to name if not named.
    std::string sinkInletId_ = ((INSTRTXT *)h.insdshead->nxti->optext)->insname;
    sinkInletId_ += ":";
    sinkInletId_ += csound->strarg2name(csound,
					(char *)0,
					Sname,
					(char *)"",
					(int) csound->GetInputArgSMask(this));
    std::vector<Inletk *> &kinlets = kinletsForCsoundsForSinkInletIds[csound][sinkInletId_];
    if (std::find(kinlets.begin(), kinlets.end(), this) == kinlets.end()) {
      kinlets.push_back(this);
    }
    // Find source outlets connecting to this.
    // Any number of sources may connect to any number of sinks.
    std::vector<std::string> &sourceOutletIds = connectionsForCsounds[csound][sinkInletId_];
    for (size_t i = 0, n = sourceOutletIds.size(); i < n; i++) {
      const std::string &sourceOutletId = sourceOutletIds[i];
      std::vector<Outletk *> &koutlets = koutletsForCsoundsForSourceOutletIds[csound][sourceOutletId];
      if (std::find(sourceOutlets->begin(), sourceOutlets->end(), &koutlets) != sourceOutlets->end()) {
	sourceOutlets->push_back(&koutlets);
      }
    }
    stdStringsForCsounds[csound].push_back(sinkInletId_);
    sinkInletId = stdStringsForCsounds[csound].back().c_str();
    return OK;
  }
  /**
   * Sum krate values from active outlets feeding this inlet.
   */
  int kontrol(CSOUND *csound)
  {
    // Zero the inlet buffer.
    *ksignal = FL(0.0);
    // Loop over the source connections...
    for (size_t sourceI = 0, sourceN = sourceOutlets->size(); 
	 sourceI < sourceN; 
	 sourceI++) {
      // Loop over the source connection instances...
      const std::vector<Outletk *> *instances = sourceOutlets->at(sourceI);
      for (size_t instanceI = 0, instanceN = instances->size(); 
	   instanceI < instanceN; 
	   instanceI++) {
	const Outletk *sourceOutlet = instances->at(instanceI);
	// Skip inactive instances.
	if (sourceOutlet->h.insdshead->actflg) {
	  *ksignal += *sourceOutlet->ksignal;
	}
      }
    }
    return OK;
  }
};

struct Outletf : public OpcodeBase<Outletf>
{
  /**
   * Output.
   */
  PVSDAT *fsignal;
  /**
   * Input.
   */
  MYFLT *Sname;
  /**
   * State.
   */
  const char *sourceOutletId;
  int init(CSOUND *csound)
  {
    // May need to convert insno to name if not named.
    std::string sourceOutletId_ = ((INSTRTXT *)h.insdshead->nxti->optext)->insname;
    sourceOutletId_ += ":";
    sourceOutletId_ += csound->strarg2name(csound,
					   (char *) 0,
					   Sname,
					   (char *)"",
					   (int) csound->GetInputArgSMask(this));
    std::vector<Outletf *> &foutlets = foutletsForCsoundsForSourceOutletIds[csound][sourceOutletId_];
    if (std::find(foutlets.begin(), foutlets.end(), this) == foutlets.end()) {
      foutlets.push_back(this);
    }
    stdStringsForCsounds[csound].push_back(sourceOutletId_);
    sourceOutletId = stdStringsForCsounds[csound].back().c_str();
    return OK;
  }
};

struct Inletf : public OpcodeBase<Inletf>
{
  /**
   * Inputs.
   */
  MYFLT *Sname;
  PVSDAT *fsignal;
  /**
   * State.
   */
  const char *sinkInletId;
  std::vector< std::vector<Outletf *> *> *sourceOutlets;
  int ksmps;
  int lastframe;
  bool fsignalInitialized;
  int init(CSOUND *csound)
  {
    ksmps = csound->GetKsmps(csound);
    lastframe = 0;
    fsignalInitialized = false;
    if (std::find(foutletVectorsForCsounds[csound].begin(), 
		  foutletVectorsForCsounds[csound].end(), 
		  sourceOutlets) == foutletVectorsForCsounds[csound].end()) {
      sourceOutlets = new std::vector< std::vector<Outletf *> *>;
      foutletVectorsForCsounds[csound].push_back(sourceOutlets);
    }
    // May need to convert insno to name if not named.
    std::string sinkInletId_ = ((INSTRTXT *)h.insdshead->nxti->optext)->insname;
    sinkInletId_ += ":";
    sinkInletId_ += csound->strarg2name(csound,
					(char *)0,
					Sname,
					(char *)"",
					(int) csound->GetInputArgSMask(this));
    std::vector<Inletf *> &finlets = finletsForCsoundsForSinkInletIds[csound][sinkInletId_];
    if (std::find(finlets.begin(), finlets.end(), this) == finlets.end()) {
      finlets.push_back(this);
    }
    // Find source outlets connecting to this.
    // Any number of sources may connect to any number of sinks.
    std::vector<std::string> &sourceOutletIds = connectionsForCsounds[csound][sinkInletId_];
    for (size_t i = 0, n = sourceOutletIds.size(); i < n; i++) {
      const std::string &sourceOutletId = sourceOutletIds[i];
      std::vector<Outletf *> &foutlets = foutletsForCsoundsForSourceOutletIds[csound][sourceOutletId];
      if (std::find(sourceOutlets->begin(), sourceOutlets->end(), &foutlets) != sourceOutlets->end()) {
	sourceOutlets->push_back(&foutlets);
      }
    }
    stdStringsForCsounds[csound].push_back(sinkInletId_);
    sinkInletId = stdStringsForCsounds[csound].back().c_str();
    return OK;
  }
  /**
   * Mix fsig values from active outlets feeding this inlet.
   */
  int audio(CSOUND *csound)
  {
    float *sink = 0;
    float *source = 0;
    CMPLX *sinkFrame = 0;
    CMPLX *sourceFrame = 0;
    // Loop over the source connections...
    for (size_t sourceI = 0, sourceN = sourceOutlets->size(); 
	 sourceI < sourceN; 
	 sourceI++) {
      // Loop over the source connection instances...
      const std::vector<Outletf *> *instances = sourceOutlets->at(sourceI);
      for (size_t instanceI = 0, instanceN = instances->size(); 
	   instanceI < instanceN; 
	   instanceI++) {
	const Outletf *sourceOutlet = instances->at(instanceI);
	// Skip inactive instances.
	if (sourceOutlet->h.insdshead->actflg) {
	  int     i;
	  int32    framesize;
	  int     test;
	  float   *fout, *fa, *fb;
	  if (!fsignalInitialized) {
	    int32 N = sourceOutlet->fsignal->N;  
	    if (UNLIKELY(sourceOutlet->fsignal == fsignal)) {
	      csound->Warning(csound, "Unsafe to have same fsig as in and out");
	    }
#ifndef OLPC
	    fsignal->sliding = 0;
	    if (sourceOutlet->fsignal->sliding) {
	      if (fsignal->frame.auxp == NULL ||
		  fsignal->frame.size < sizeof(MYFLT) * csound->ksmps * (N + 2))
		csound->AuxAlloc(csound, (N + 2) * sizeof(MYFLT) * csound->ksmps,
				 &fsignal->frame);
	      fsignal->NB = sourceOutlet->fsignal->NB;
	      fsignal->sliding = 1;
	    } 
	    else
#endif
	      if (fsignal->frame.auxp == NULL ||
		  fsignal->frame.size < sizeof(float) * (N + 2)) {
		csound->AuxAlloc(csound, (N + 2) * sizeof(float), &fsignal->frame);
	      }
	    fsignal->N = N;
	    fsignal->overlap = sourceOutlet->fsignal->overlap;
	    fsignal->winsize = sourceOutlet->fsignal->winsize;
	    fsignal->wintype = sourceOutlet->fsignal->wintype;
	    fsignal->format = sourceOutlet->fsignal->format;
	    fsignal->framecount = 1;
	    lastframe = 0;
	    if (UNLIKELY(!(fsignal->format == PVS_AMP_FREQ) ||
			 (fsignal->format == PVS_AMP_PHASE)))
	      return csound->InitError(csound, Str("inletf: signal format "
						   "must be amp-phase or amp-freq."));
	    fsignalInitialized = true;
	  }
#ifndef OLPC
	  if (fsignal->sliding) {
	    for (size_t frameI = 0; frameI < ksmps; frameI++) {
	      sinkFrame = (CMPLX*) fsignal->frame.auxp + (fsignal->NB * frameI);
	      sourceFrame = (CMPLX*) sourceOutlet->fsignal->frame.auxp + (fsignal->NB * frameI);
	      for (size_t binI = 0, binN = fsignal->NB; binI < binN; i++) {
		if (sourceFrame[binI].re > sinkFrame[binI].re) 
		  sinkFrame[binI] = sourceFrame[binI];
		}
	      }
	    }
	  } else {
#endif
	    sink = (float *)fsignal->frame.auxp;
	    source = (float *)sourceOutlet->fsignal->frame.auxp;
	    if (lastframe < fsignal->framecount) {
	      for (size_t binI = 0, binN = fsignal->N + 2;
		   binI < binN;
		   binI += 2) {
		if (source[binI] > sink[binI]) {
		  source[binI] = sink[binI];
		  source[binI + 1] = sink[binI + 1];
		}
	      }
	      fsignal->framecount = lastframe = sourceOutlet->fsignal->framecount;
	    }
#ifndef OLPC
	  }  
#endif
      }
    }
    return OK;
  }
};

struct Connect : public OpcodeBase<Connect>
{
  /**
   * Inputs.
   */
  MYFLT *Source;
  MYFLT *Soutlet;
  MYFLT *Sink;
  MYFLT *Sinlet;
  int init(CSOUND *csound)
  {
    std::string sourceOutletId = csound->strarg2name(csound,
						     (char *) 0,
						     Source,
						     (char *)"",
						     (int) csound->GetInputArgSMask(this));
    sourceOutletId += ":";
    sourceOutletId += csound->strarg2name(csound,
					  (char *) 0,
					  Soutlet,
					  (char *)"",
					  (int) csound->GetInputArgSMask(this));
    std::string sinkInletId = csound->strarg2name(csound,
						  (char *) 0,
						  Sink,
						  (char *)"",
						  (int) csound->GetInputArgSMask(this));
    sinkInletId += ":";
    sinkInletId += csound->strarg2name(csound,
				       (char *) 0,
				       Sinlet,
				       (char *)"",
				       (int) csound->GetInputArgSMask(this));
    connectionsForCsounds[csound][sinkInletId].push_back(sourceOutletId);
    return OK;
  }
};

struct AlwaysOn : public OpcodeBase<AlwaysOn>
{
  /**
   * Inputs.
   */
  MYFLT *Sinstrument;
  MYFLT *argums[VARGMAX];
  /**
   * State.
   */
  EVTBLK evtblk;
  int init(CSOUND *csound)
  {
    std::string source = csound->strarg2name(csound,
					     (char *) 0,
					     Sinstrument,
					     (char *)"",
					     (int) csound->GetInputArgSMask(this));
    evtblk.opcod = 'i';
    evtblk.strarg = 0;
    evtblk.p[0] = FL(0.0);
    evtblk.p[1] = h.insdshead->p1;           
    evtblk.p[2] = evtblk.p2orig = FL(0.0);                   
    evtblk.p[3] = evtblk.p3orig = -1.0;
    if (csound->GetInputArgSMask(this)) {  
      evtblk.p[1] = SSTRCOD;
      evtblk.strarg = (char *) Sinstrument;
    }
    int n = csound->GetInputArgCnt(this);
    evtblk.pcnt = (int16) n;
    for (size_t fpI = 4, argumsI = 0; argumsI < n; fpI++, argumsI++) {
      evtblk.p[fpI] = *argums[argumsI];
    }
    csound->insert_score_event(csound, &evtblk, FL(0.0));	
    return OK;
  }
};

struct FtGenOnce : public OpcodeBase<FtGenOnce>
{
  /**
   * Outputs.
   */
  MYFLT *ifno;
  /**
   * Inputs.
   */
  MYFLT *p1;
  MYFLT *p2;
  MYFLT *p3; 
  MYFLT *p4; 
  MYFLT *p5;
  MYFLT *argums[VARGMAX];
  /** 
   * State is external and global.
   */
  int init(CSOUND *csound)
  {
    // Default output.
    *ifno = FL(0.0);
    EVTBLK evtblk;
    std::memset(&evtblk, 0, sizeof(EVTBLK));
    // No need to compare this one, always has the same value.
    evtblk.opcod = 'f';
    evtblk.strarg = 0;
    evtblk.p[0] = FL(0.0);
    evtblk.p[1] = *p1;                                     
    evtblk.p[2] = evtblk.p2orig = FL(0.0);                   
    evtblk.p[3] = evtblk.p3orig = -1.0;
    evtblk.p[4] = *p4;
    int n = 0;
    if (csound->GetInputArgSMask(this)) {  
      n = (int) evtblk.p[4];
      evtblk.p[5] = SSTRCOD;
      if (n < 0) {
	n = -n;
      }
      // Only GEN 1, 23, 28, or 43 can take strings.
      switch (n) {                      
      case 1:
      case 23:
      case 28:
      case 43:
	evtblk.strarg = (char *) p5;
	break;
      default:
	return csound->InitError(csound, Str("ftgen string arg not allowed"));
      }
    }
    else {
      evtblk.p[5] = *p5;                                  
    }
    evtblk.pcnt = (int16) csound->GetInputArgCnt(this);
    for (size_t pfieldI = 6; pfieldI < evtblk.pcnt; pfieldI++) {
      evtblk.p[pfieldI] = *argums[pfieldI - 5];
    }
    // If the arguments have not been used before for this instance of Csound,
    // create a new function table and store the arguments and table number.
    if(functionTablesForCsoundsForEvtblks[csound].find(evtblk) == functionTablesForCsoundsForEvtblks[csound].end()) {
      FUNC *func = 0;
      n = csound->hfgens(csound, &func, &evtblk, 1);       
      if (UNLIKELY(n != 0)) {
	return csound->InitError(csound, Str("ftgen error"));
      }
      if (func) {
	*ifno = (MYFLT) func->fno;                     
	functionTablesForCsoundsForEvtblks[csound][evtblk] = func->fno;
      }
      csound->Message(csound, "ftgenonce: created new func: %d\n", func->fno);
    } else {
      *ifno = functionTablesForCsoundsForEvtblks[csound][evtblk];
      csound->Message(csound, "ftgenonce: re-using existing func: %f\n", *ifno);
    }
    return OK;
  }
};

extern "C"
{
  static OENTRY oentries[] = {
    {
      (char *)"signalflowgraph",
      sizeof(SignalFlowGraph),
      1,
      (char *)"",
      (char *)"",
      (SUBR)&SignalFlowGraph::init_,
      0,
      0,
    },
    {
      (char *)"outleta",
      sizeof(Outleta),
      5,
      (char *)"",
      (char *)"Sa",
      (SUBR)&Outleta::init_,
      0,
      (SUBR)&Outleta::audio_
    },
    {
      (char *)"inleta",
      sizeof(Inleta),
      5,
      (char *)"a",
      (char *)"S",
      (SUBR)&Inleta::init_,
      0,
      (SUBR)&Inleta::audio_
    },
    {
      (char *)"outletk",
      sizeof(Outletk),
      3,
      (char *)"",
      (char *)"Sk",
      (SUBR)&Outletk::init_,
      (SUBR)&Outletk::kontrol_,
      0
    },
    {
      (char *)"inletk",
      sizeof(Inletk),
      3,
      (char *)"k",
      (char *)"S",
      (SUBR)&Inletk::init_,
      (SUBR)&Inletk::kontrol_,
      0
    },
    {
      (char *)"outletf",
      sizeof(Outletf),
      5,
      (char *)"",
      (char *)"Sf",
      (SUBR)&Outletf::init_,
      0,
      (SUBR)&Outletf::audio_
    },
    {
      (char *)"inletf",
      sizeof(Inletf),
      5,
      (char *)"f",
      (char *)"S",
      (SUBR)&Inletf::init_,
      0,
      (SUBR)&Inletf::audio_
    },
    { 
      (char *)"connect", 
      sizeof(Connect),     
      1,  
      (char *)"",  
      (char *)"SSSS", 
      (SUBR)&Connect::init_, 
      0, 
      0 
    },
    { 
      (char *)"alwayson", 
      sizeof(AlwaysOn),     
      1,  
      (char *)"i",  
      (char *)"Tm", 
      (SUBR)&AlwaysOn::init_, 
      0, 
      0 
    },
    { 
      (char *)"ftgenonce", 
      sizeof(FtGenOnce),     
      1,  
      (char *)"i",  
      (char *)"iiiiTm", 
      (SUBR)&FtGenOnce::init_, 
      0, 
      0 
    },
    { 0, 0, 0, 0, 0, (SUBR) 0, (SUBR) 0, (SUBR) 0 }
  };

  PUBLIC int csoundModuleCreate(CSOUND *csound)
  {
    return 0;
  }

  PUBLIC int csoundModuleInit(CSOUND *csound)
  {
    OENTRY *ep = (OENTRY *)&(oentries[0]);
    int  err = 0;
    while (ep->opname != 0) {
      err |= csound->AppendOpcode(csound,
				  ep->opname, 
				  ep->dsblksiz, 
				  ep->thread,
				  ep->outypes, 
				  ep->intypes,
				  (int (*)(CSOUND *, void*)) ep->iopadr,
				  (int (*)(CSOUND *, void*)) ep->kopadr,
				  (int (*)(CSOUND *, void*)) ep->aopadr);
      ep++;
    }
    return err;
  }

  PUBLIC int csoundModuleDestroy(CSOUND *csound)
  {
#pragma omp critical
    {
      csound->Message(csound, "signalflowgraph: CsoundModuleDestroy(0x%x)\n", csound);
      aoutletsForCsoundsForSourceOutletIds[csound].clear();
      koutletsForCsoundsForSourceOutletIds[csound].clear();
      foutletsForCsoundsForSourceOutletIds[csound].clear();
      ainletsForCsoundsForSinkInletIds[csound].clear();
      kinletsForCsoundsForSinkInletIds[csound].clear();
      finletsForCsoundsForSinkInletIds[csound].clear();
      connectionsForCsounds[csound].clear();
      functionTablesForCsoundsForEvtblks[csound].clear();
      for (size_t i = 0, n = aoutletVectorsForCsounds[csound].size(); i < n; i++) {
	delete aoutletVectorsForCsounds[csound][i];
      }
      aoutletVectorsForCsounds[csound].clear();
      for (size_t i = 0, n = koutletVectorsForCsounds[csound].size(); i < n; i++) {
	delete koutletVectorsForCsounds[csound][i];
      }
      koutletVectorsForCsounds[csound].clear();
      for (size_t i = 0, n = foutletVectorsForCsounds[csound].size(); i < n; i++) {
	delete foutletVectorsForCsounds[csound][i];
      }
      foutletVectorsForCsounds[csound].clear();
      stdStringsForCsounds[csound].clear();
    }
    return 0;
  }
}  
