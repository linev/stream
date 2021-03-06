#include "base/ProcMgr.h"
#include "hadaq/HldProcessor.h"
#include "hadaq/TdcProcessor.h"
#include "hadaq/TrbProcessor.h"

#include <cstdlib>

void first()
{
   base::ProcMgr::instance()->SetRawAnalysis(true);
   // In triggered mode create output data are created and calls processors from second.C
   // base::ProcMgr::instance()->SetTriggeredAnalysis(true);

   // configure sorting of created folders, default is on
   base::ProcMgr::instance()->SetSortedOrder(true);

   // all new instances get this value
   base::ProcMgr::instance()->SetHistFilling(4);

   // all new instances get this value
   // 0 - no histograms
   // 1 - basic histograms in HLD/TRB
   // 2 - generic histograms for each TDC
   // 3 - include histograms for each active TDC channel
   // 4 - also special time reference histograms for channels (when configured)
   base::ProcMgr::instance()->SetHistFilling(4);

   // this limits used for liner calibrations when nothing else is available
   hadaq::TdcMessage::SetFineLimits(31, 491);

   // Enable 1 or disable 0 errors logging from following enumeration
   //  { 0x1: errNoHeader, 0x2: errChId, 0x4: errEpoch, 0x8: errFine,
   //    0x10: err3ff, 0x20: errCh0, 0x40: errMismatchDouble, 0x80: errUncknHdr, 0x100: errDesignId, 0x200: errMisc }
   // hadaq::TdcProcessor::SetErrorMask(0xffffff);

   // Create histograms for all channels immediately - even if channel never appear in data
   // hadaq::TdcProcessor::SetAllHistos(true);

   // Configure window (in nanoseconds), where time stamps from 0xD trigger will be accepted for calibration
   // hadaq::TdcProcessor::SetTriggerDWindow(-25, 50);

   // enable usage of D trigger in reference histograms
   // hadaq::TdcProcessor::SetUseDTrigForRef(true);

   // configure ToT calibration parameters
   // first - minimal number of counts in ToT histogram
   // second - maximal RMS value
   hadaq::TdcProcessor::SetToTCalibr(100, 0.2);

   // default channel numbers and edges usage
   // 1 - use only rising edge, falling edge is ignore
   // 2 - falling edge enabled and fully independent from rising edge
   // 3 - falling edge enabled and uses calibration from rising edge
   // 4 - falling edge enabled and common statistic is used for calibration
   hadaq::TrbProcessor::SetDefaults(65, 1);

   // [min..max] range for TDC ids
   hadaq::TrbProcessor::SetTDCRange(0x940, 0x94F);

   // [min..max] range for HUB ids
   hadaq::TrbProcessor::SetHUBRange(0x8200, 0x82FF);

   // when first argument true - TRB/TDC will be created on-the-fly
   // second parameter is function name, called after elements are created
   hadaq::HldProcessor* hld = new hadaq::HldProcessor(true, "after_create");

   const char* calname = getenv("CALNAME");
   if ((calname==0) || (*calname==0)) calname = "cal/test_";
   const char* calmode = getenv("CALMODE");
   int cnt = (calmode && *calmode) ? atoi(calmode) : -1;
   const char* caltrig = getenv("CALTRIG");
   unsigned trig = (caltrig && *caltrig) ? atoi(caltrig) : 0xD;
   const char* uset = getenv("USETEMP");
   unsigned use_temp = 0; // 0x80000000;
   if ((uset!=0) && (*uset!=0) && (strcmp(uset,"1")==0)) use_temp = 0x80000000;

   printf("HLD configure calibration calfile:%s  cnt:%d trig:%X temp:%X\n", calname, cnt, trig, use_temp);

   // first parameter if filename  prefix for calibration files
   //     and calibration mode (empty string - no file I/O)
   // second parameter is hits count for autocalibration
   //     0 - only load calibration
   //    -1 - accumulate data and store calibrations only at the end
   //    -77 - accumulate data and produce liner calibrations only at the end
   //    >0 - automatic calibration after N hits in each active channel
   //         if value ends with 77 like 10077 linear calibration will be calculated
   //    >1000000000 - automatic calibration after N hits only once, 1e9 excluding
   // third parameter is trigger type mask used for calibration
   //   (1 << 0xD) - special 0XD trigger with internal pulser, used also for TOT calibration
   //    0x3FFF - all kinds of trigger types will be used for calibration (excluding 0xE and 0xF)
   //   0x80000000 in mask enables usage of temperature correction
   hld->ConfigureCalibration(calname, cnt, (1 << trig) | use_temp);

   // only accept trigger type 0x1 when storing file
   // new hadaq::HldFilter(0x1);

   // create ROOT file store
   // base::ProcMgr::instance()->CreateStore("td.root");

   // 0 - disable store
   // 1 - std::vector<hadaq::TdcMessageExt> - includes original TDC message
   // 2 - std::vector<hadaq::MessageFloat>  - compact form, without channel 0, stamp as float (relative to ch0)
   // 3 - std::vector<hadaq::MessageDouble> - compact form, with channel 0, absolute time stamp as double
   base::ProcMgr::instance()->SetStoreKind(3);

   // when configured as output in DABC, one specifies:
   // <OutputPort name="Output2" url="stream://file.root?maxsize=5000&kind=3"/>
}

// extern "C" required by DABC to find function from compiled code
// function called once all TDC instances are created, here one can configure them

extern "C" void after_create(hadaq::HldProcessor* hld)
{
   printf("Called after all sub-components are created\n");

   if (hld==0) return;

   for (unsigned k=0;k<hld->NumberOfTRB();k++) {
      hadaq::TrbProcessor* trb = hld->GetTRB(k);
      if (trb==0) continue;
      printf("Configure %s!\n", trb->GetName());
      trb->SetPrintErrors(10);
   }

   for (unsigned k=0;k<hld->NumberOfTDC();k++) {
      hadaq::TdcProcessor* tdc = hld->GetTDC(k);
      if (tdc==0) continue;

      printf("Configure %s!\n", tdc->GetName());

      // configure 0xD trigger width and hmin/hmax histogram range for 0xD trigger ToT
      tdc->SetToTRange(20, 15., 60.);

      // tdc->SetUseLastHit(true);
      for (unsigned nch=2;nch<tdc->NumChannels();nch++)
        tdc->SetRefChannel(nch, 1, 0xffff, 6000,  -20., 40.);

   }
}


