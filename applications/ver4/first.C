#include "base/ProcMgr.h"
#include "hadaq/HldProcessor.h"
#include "hadaq/TdcProcessor.h"
#include "hadaq/TrbProcessor.h"


class ClearProcessor : public base::EventProc {
protected:
   int fLimit;
   int fCounter;
public:

   ClearProcessor(int limit = 10000) : base::EventProc(), fLimit(limit), fCounter(0) {}
   virtual ~ClearProcessor() {}

   virtual bool Process(base::Event* ev)
   {
      if (fCounter++ >= fLimit) {
         fCounter = 0;
         base::ProcMgr::instance()->ClearAllHistograms();
      }
      return true;
   }
};


void first()
{
   // base::ProcMgr::instance()->SetRawAnalysis(true);
   base::ProcMgr::instance()->SetTriggeredAnalysis(true);

   // all new instances get this value
   base::ProcMgr::instance()->SetHistFilling(4);

   // this limits used for liner calibrations when nothing else is available
   hadaq::TdcMessage::SetFineLimits(28, 350);

   // clear all histograms every 10000 events
   // new ClearProcessor(10000);

   // default channel numbers and edges mask
   // 1 - use only rising edge, falling edge is ignore
   // 2   - falling edge enabled and fully independent from rising edge
   // 3   - falling edge enabled and uses calibration from rising edge
   // 4   - falling edge enabled and common statistic is used for calibration
   hadaq::TrbProcessor::SetDefaults(33, 2);

   // enable analysis of 0xD trigger for refs
   hadaq::TdcProcessor::SetUseDTrigForRef(true);

   // [min..max] range for TDC ids
   hadaq::TrbProcessor::SetTDCRange(0x1600, 0x16FF);

   // [min..max] range for HUB ids
   hadaq::TrbProcessor::SetHUBRange(0x8800, 0x88FF);

   // when first argument true - TRB/TDC will be created on-the-fly
   // second parameter is function name, called after elements are created
   hadaq::HldProcessor* hld = new hadaq::HldProcessor(true, "after_create");

   // first parameter if filename  prefix for calibration files
   //     and calibration mode (empty string - no file I/O)
   // second parameter is hits count for autocalibration
   //     0 - only load calibration
   //    -1 - accumulate data and store calibrations only at the end
   //    -77 - accumulate data and store linear calibrations only at the end
   //    >0 - automatic calibration after N hits in each active channel
   //         if value ends with 77 like 10077 linear calibration will be calculated
   //    >1000000000 - automatic calibration after N hits only once, 1e9 excluding
   // third parameter is trigger type mask used for calibration
   //   (1 << 0xD) - special 0XD trigger with internal pulser, used also for TOT calibration
   //    0x3FFF - all kinds of trigger types will be used for calibration (excluding 0xE and 0xF)
   //   0x80000000 in mask enables usage of temperature correction
   hld->ConfigureCalibration("test", 0, (1 << 0xD));

   // only accept trigger type 0x1 when storing file
   // new hadaq::HldFilter(0x1);

   // create ROOT file store
   // base::ProcMgr::instance()->CreateStore("td.root");

   // 0 - disable store
   // 1 - std::vector<hadaq::TdcMessageExt> - includes original TDC message
   // 2 - std::vector<hadaq::MessageFloat>  - compact form, without channel 0, stamp as float (relative to ch0)
   // 3 - std::vector<hadaq::MessageDouble> - compact form, with channel 0, absolute time stamp as double
   base::ProcMgr::instance()->SetStoreKind(2);

   // when configured as output in DABC, one specifies:
   // <OutputPort name="Output2" url="stream://file.root?maxsize=5000&kind=3"/>
}

// extern "C" required by DABC to find function from compiled code

extern "C" void after_create(hadaq::HldProcessor* hld)
{
   printf("Called after all sub-components are created\n");

   if (!hld) return;

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

      if (tdc->GetID() == 0x16f7) {
         tdc->SetRefChannel(12, 14, 0xffff, 2000,  -10., 10.);
         tdc->EnableRefCondPrint(12, 0.6, 0.65, 100);
      }

      // tdc->SetUseLastHit(true);
      // do not configure ref channels, otherwise all histograms will be created
      //for (unsigned nch=2;nch<tdc->NumChannels();nch++)
      //  tdc->SetRefChannel(nch, 1, 0xffff, 2000,  -10., 10.);
   }
}


