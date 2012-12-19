#ifndef HADAQ_DEFINES_H
#define HADAQ_DEFINES_H

#include <stdint.h>

/*
//Description of the Event Structure
//
//An event consists of an event header and of varying number of subevents, each with a subevent header. The size of the event header is fixed to 0x20 bytes
//
//Event header
//evtHeader
//evtSize   evtDecoding    evtId    evtSeqNr    evtDate  evtTime  runNr    expId
//
//    * evtSize - total size of the event including the event header, it is measured in bytes.
//    * evtDecoding - event decoding type: Tells the analysis the binary format of the event data, so that it can be handed to a corresponding routine for unpacking into a software usable data structure. For easier decoding of this word, the meaning of some bits is already predefined:
//      evtDecoding
//      msB    ---   ---   lsB
//      0   alignment   decoding type  nonzero
//          o The first (most significant) byte is always zero.
//          o The second byte contains the alignment of the subevents in the event. 0 = byte, 1 = 16 bit word, 2 = 32 bit word...
//          o The remaining two bytes form the actual decoding type, e.g. fixed length, zero suppressed etc.
//          o The last byte must not be zero, so the whole evtDecoding can be used to check for correct or swapped byte order.
//
//It is stated again, that the whole evtDecoding is one 32bit word. The above bit assignments are merely a rule how to select this 32bit numbers.
//
//    * evtId - event identifier: Tells the analysis the semantics of the event data, e.g. if this is a run start event, data event, simulated event, slow control data, end file event, etc..
//      evtId
//      31  30 - 16  15 - 12  11 - 8   5 - 7    4  3- 0
//      error bit    reserved    version  reserved    MU decision    DS flag  ID
//          o error bit - set if one of the subsystems has set the error bit
//          o version - 0 meaning of event ID before SEP03; 1 event ID after SEP03
//          o MU decision - 0 = negative LVL2 decision; >0 positive LVL2 decision
//            MU trigger algo result
//            1   negative decision
//            2   positive decision
//            3   positive decision due to too many leptons or dileptons
//            4   reserved
//            5   reserved
//          o DS flag - LVL1 downscaling flag; 1 = this event is written to the tape independent on the LVL2 trigger decision
//          o ID - defines the trigger code
//            ID before SEP03    description
//            0   simulation
//            1   real
//            2,3,4,5,6,7,8,9    calibration
//            13  beginrun
//            14  endrun
//
//            ID after SEP03  description
//            0   simulation
//            1,2,3,4,5    real
//            7,9    calibration
//            1   real1
//            2   real2
//            3   real3
//            4   real4
//            5   real5
//            6   special1
//            7   offspill
//            8   special3
//            9   MDCcalibration
//            10  special5
//            13  beginrun
//            14  endrun
//    * evtSeqNr - event number: This is the sequence number of the event in the file. The pair evtFileNr/evtSeqNr
//
//is unique throughout all events ever acquired by the system.
//
//    * evtDate - date of event assembly (filled by the event builder, rough precision):
//      evtDate   ISO-C date format
//      msB    ---   ---   lsB
//      0   year  month    day
//
//   1. The first (most significant) byte is zero
//   2. The second byte contains the years since 1900
//   3. The third the months since January [0-11]
//   4. The last the day of the month [1-31]
//
//    * evtTime - time of assembly (filled by the event builder, rough precision):
//      evtTime   ISO-C time format
//      msB    ---   ---   lsB
//      0   hour  minute   second
//
//   1. The first (most significant) byte is zero
//   2. The second byte contains the hours since midnight [0-23]
//   3. The third the minutes after the hour [0-59]
//   4. The last the seconds after the minute [0-60]
//
//    * runNr - file number: A unique number assigned to the file. The runNr is used as key for the RTDB.
//    * evtPad - padding: Makes the event header a multiple of 64 bits long.
*/

#pragma pack(push, 1)

namespace hadaq {

   struct RawEvent {

      protected:

      //    hide all members from user while they can be swapped
         uint32_t evtSize;
         uint32_t evtDecoding;
         uint32_t evtId;
         uint32_t evtSeqNr;
         uint32_t evtDate;
         uint32_t evtTime;
         uint32_t runNr;
         uint32_t evtPad;

      public:

         RawEvent() {}
         ~RawEvent() {}

         /** msb of decode word is always non zero...? */
         bool IsSwapped() const  { return  evtDecoding > 0xffffff; }

         /** swapsave access to any data stolen from hadtu.h */
         uint32_t Value(const uint32_t *member) const
         {
            if (!IsSwapped()) return *member;
            uint32_t tmp;
            ((uint8_t *) &tmp)[0] = ((uint8_t *) member)[3];
            ((uint8_t *) &tmp)[1] = ((uint8_t *) member)[2];
            ((uint8_t *) &tmp)[2] = ((uint8_t *) member)[1];
            ((uint8_t *) &tmp)[3] = ((uint8_t *) member)[0];
            return tmp;
         }

         uint32_t GetSize() const { return Value(&evtSize); }

         uint32_t GetId() const { return Value(&evtId); }

         uint32_t GetSeqNr() const { return Value(&evtSeqNr); }
   };

/*
//Subevent
//
//Every event contains zero to unspecified many subevents. As an empty event is allowed, data outside of any subevent are not allowed. A subevents consists out of a fixed size subevent header and a varying number of data words.
//
//    * The subevent header
//      subEvtHeader
//      subEvtSize   subEvtDecoding    subEvtId    subEvtTrigNr
//          o subEvtSize - size of the subevent: This includes the the subevent header, it is measured in bytes.
//          o subEvtDecoding - subevent decoding type: Tells the analysis the binary format of the subevent data, so that it can be handed to a corresponding routine for unpacking into a software usable data structure. For easier decoding of this word, the meaning of some bits is already predefined:
//            subEvtDecoding
//            msB    ---   ---   lsB
//            0   data type   decoding type  nonzero
//                + The first (most significant) byte is always zero
//                + The second byte contains the word length of the subevent data. 0 = byte, 1 = 16 bit word, 2 = 32 bit word...
//                + The remaining two bytes form the actual decoding type, e.g. fixed length, zero suppressed etc.
//                + The last byte must not be zero, so the whole subEvtDecoding can be used to check for correct or swapped byte order. It is stated again, that the whole subEvtDecoding is one 32bit word. The above bit assignments are merely a rule how to select this 32bit numbers.
//          o subEvtId - subevent identifier: Tells the analysis the semantics of the subevent data, e.g. every subevent builder may get its own subEvtId. So the data structure can be analyzed by the corresponding routine after unpacking.
//            1-99   DAQ
//            100-199   RICH
//            200-299   MDC
//            300-399   SHOWER
//            400-499   TOF
//            500-599   TRIG
//            600-699   SLOW
//            700-799   TRB_RPC  common TRB, but contents is RPC
//            800-899   TRB_HOD  pion-hodoscope
//            900-999   TRB_FW   forward wall
//            1000-1099    TRB_START   start detector
//            1100-1199    TRB_TOF  TOF detector
//            1200-1299    TRB RICH    RICH detector
//
//Additionally, all subEvtIds may have the MSB set. This indicates a sub event of the corresponding id that contains broken data (e.g. parity check failed, sub event was too long etc.).
//
//    *
//          o subEvtTrigNr - subevent Trigger Number: This is the event tag that is produced by the trigger system and is used for checking the correct assembly of several sub entities. In general, this number is not counting sequentially. The lowest significant byte represents the trigger tag generated by the CTU and has to be same for all detector subsystems in one event. The rest is filled by the EB.
//    * The data words: The format of the data words (word length, compression algorithm, sub-sub-event format) is defined by the subEvtDecoding and apart from this it is completely free. The meaning of the data words (detector, geographical position, error information) is defined by the subEvtId and apart from this it is completely unknown to the data acquisition system.
*/

   struct RawSubevent {

      protected:

         uint32_t subEvtSize;
         uint32_t subEvtDecoding;
         uint32_t subEvtId;
         uint32_t subEvtTrigNr;

      public:

         RawSubevent() {}
         ~RawSubevent() {}

         /** msb of decode word is always non zero...? */
         bool IsSwapped() const { return  (subEvtDecoding > 0xffffff); }

         unsigned Alignment() const { return 1 << ( GetDecoding() >> 16 & 0xff); }

         /** swap-save access to any data stolen from hadtu.h */
         uint32_t Value(const uint32_t *member) const
         {
            return IsSwapped() ? ((((uint8_t *) member)[0] << 24) |
                                  (((uint8_t *) member)[1] << 16) |
                                  (((uint8_t *) member)[2] << 8) |
                                  (((uint8_t *) member)[3])) : *member;
         }

         uint32_t GetSize() const { return Value(&subEvtSize); }

         uint32_t GetDecoding() const { return Value(&subEvtDecoding); }

         uint32_t GetId() const { return Value(&subEvtId); }

         uint32_t GetTrigNr() const { return Value(&subEvtTrigNr); }

         /** Return pointer on data by index - user should care itself about swapping */
         uint32_t* GetDataPtr(unsigned indx) const
         {
            return (uint32_t*) (this) + sizeof(RawSubevent)/sizeof(uint32_t) + indx;
         }

         /** swap-save access to any data. stolen from hadtu.h */
         uint32_t Data(unsigned idx) const
         {
            const void* my = (char*) (this) + sizeof(RawSubevent);

            switch (Alignment()) {
               case 4:
                  return Value((uint32_t *) my + idx);

               case 2: {
                  uint16_t tmp = ((uint16_t *) my)[idx];

                  if (IsSwapped()) tmp = ((tmp >> 8) & 0xff) | ((tmp << 8) & 0xff00);

                  return tmp;
               }
            }

            return ((uint8_t*) my)[idx];
         }

         void CopyDataTo(void* buf, unsigned indx, unsigned datalen)
         {
            if (buf==0) return;
            while (datalen-- > 0) {
               *((uint32_t*) buf) = Data(indx++);
               buf = ((uint32_t*) buf) + 1;
            }
         }
   };

}

#pragma pack(pop)

#endif
