/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGEvent_H
#define CYGEvent_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGEvent                                                                   //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include <RConfig.h>
#include <vector>
#include <TObject.h>
#include <TClass.h>
#include <TClonesArray.h>
#include "include/util/Waveform.h"
#include "include/util/Picture.h"
#include "ROMEString.h"

// Equality operator.
// Warning! All fields may not be compared.
class CYGEvent;
Bool_t operator==(const CYGEvent &lhs, const CYGEvent &rhs);
inline Bool_t operator!=(const CYGEvent &lhs, const CYGEvent &rhs) { return !(lhs == rhs); }

class CYGEvent : public TObject
{
protected:
   Waveform*   * DGTZWaveform;           // Sense wire waveforms
   Int_t         DGTZWaveformSize;       // ! number of elements of DGTZWaveform
   Picture*    * CamPicture;             // Camera pictures
   Int_t         CamPictureSize;         // ! number of elements of CamPicture
   TClonesArray* PMTWaveform;            // Analog waveforms
   Int_t         EventTime;              // event Unix time
   Bool_t        fModified;              //! Modified Folder Flag

private:
   CYGEvent(const CYGEvent &c); // not implemented

public:
   CYGEvent(Int_t EventTime_value=0 );

   virtual ~CYGEvent();

   // Warning! All fields may not be compared.
   friend Bool_t operator==(const CYGEvent &lhs, const CYGEvent &rhs);
   friend Bool_t operator!=(const CYGEvent &lhs, const CYGEvent &rhs);

   // Warning! All fields may not be copied.
   CYGEvent& operator=(const CYGEvent &c);

   Waveform*      GetDGTZWaveformAt(Int_t indx) const;
   Waveform*    * GetDGTZWaveform() const           { return DGTZWaveform;           }
   Int_t          GetDGTZWaveformSize() const           { return DGTZWaveformSize;           }
   Picture*       GetCamPictureAt(Int_t indx) const;
   Picture*     * GetCamPicture() const             { return CamPicture;             }
   Int_t          GetCamPictureSize() const             { return CamPictureSize;             }
   Waveform     * GetPMTWaveformAt(Int_t indx) const;
   TClonesArray*  GetPMTWaveform() const            { return PMTWaveform;            }
   Int_t          GetPMTWaveformSize() const            { return PMTWaveform->GetEntriesFast();            }
   Int_t          GetEventTime() const              { return EventTime;              }

   virtual Bool_t         isModified();

   void SetDGTZWaveformAt          (Int_t indx,Waveform*     DGTZWaveform_value);
   void SetDGTZWaveform          (Waveform*   * DGTZWaveform_value          ) { DGTZWaveform = DGTZWaveform_value          ;           SetModified(true); }
   void SetDGTZWaveformSize(Int_t number);
   void SetCamPictureAt            (Int_t indx,Picture*      CamPicture_value);
   void SetCamPicture            (Picture*    * CamPicture_value            ) { CamPicture = CamPicture_value            ;             SetModified(true); }
   void SetCamPictureSize(Int_t number);
   void SetPMTWaveformSize(Int_t number);
   void SetEventTime             (Int_t         EventTime_value             ) { EventTime              = EventTime_value;              SetModified(true); }

   void AddEventTime             (Int_t         EventTime_value             ) { EventTime              += EventTime_value;              SetModified(true); }

   virtual void SetModified              (Bool_t        modified              ) { fModified               = modified;              }

   virtual void ResetModified();

   void UpdateVariableSize();

   void SetAll( Int_t EventTime_value=0 );

   virtual void Reset();

   void Print(Option_t* option) const;

private:
   Bool_t DGTZWaveformBoundsOk(const char* where, Int_t at) const;
   Bool_t DGTZWaveformOutOfBoundsError(const char* where, Int_t i) const;
   Bool_t CamPictureBoundsOk(const char* where, Int_t at) const;
   Bool_t CamPictureOutOfBoundsError(const char* where, Int_t i) const;
   Bool_t PMTWaveformBoundsOk(const char* where, Int_t at) const;
   Bool_t PMTWaveformOutOfBoundsError(const char* where, Int_t i) const;


   ClassDef(CYGEvent,1)
};

#endif   // CYGEvent_H
