/******************************************************************************
 *         ***  This file will be overwritten by the ROMEBuilder  ***         *
 *          ***      Don't make manual changes to this file      ***          *
 ******************************************************************************/

#ifndef CYGMidasDAQ_H
#define CYGMidasDAQ_H

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// CYGMidasDAQ                                                                //
//                                                                            //
// This class implements the midas odb access for CYGAnalyzer.                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////



#include "ROMEMidasDAQ.h"

// Bank Structures

// Hot Links Structures
typedef struct{
   Int_t Active; //! Task Active
} ReadDataHotLinks; // Task ReadData
typedef struct{
   Int_t Active; //! Task Active
} WriteDataHotLinks; // Task WriteData
typedef struct{
} GSPHotLinks; // Global Steering Parameters
typedef struct{
   Int_t Active; //! Tab Active
} EventDisplayHotLinks; // Tab EventDisplay
typedef struct{
   Int_t Active; //! Tab Active
} FeaturesHotLinks; // Tab Features

class CYGMidasDAQ : public ROMEMidasDAQ
{
   // Bank Fields
   DWORD* fTDC0BankPointer;  //! Pointer to the TDC0 Bank
   DWORD* fLastTDC0BankPointer;  //! Pointer to the TDC0 Bank of the last event
   DWORD fTDC0BankLength;   //! Length  of the TDC0 Bank
   DWORD fLastTDC0BankLength;   //! Length  of the TDC0 Bank of the last event
   Bool_t  fTDC0BankExists;   //! Exist Flags of the TDC0 Bank
   Bool_t  fLastTDC0BankExists;   //! Exist Flags of the TDC0 Bank of the last event
   Bool_t  fTDC0BankActive;   //! Active Flags of the TDC0 Bank
   float* fDIG0BankPointer;  //! Pointer to the DIG0 Bank
   float* fLastDIG0BankPointer;  //! Pointer to the DIG0 Bank of the last event
   DWORD fDIG0BankLength;   //! Length  of the DIG0 Bank
   DWORD fLastDIG0BankLength;   //! Length  of the DIG0 Bank of the last event
   Bool_t  fDIG0BankExists;   //! Exist Flags of the DIG0 Bank
   Bool_t  fLastDIG0BankExists;   //! Exist Flags of the DIG0 Bank of the last event
   Bool_t  fDIG0BankActive;   //! Active Flags of the DIG0 Bank
   float* fDIG1BankPointer;  //! Pointer to the DIG1 Bank
   float* fLastDIG1BankPointer;  //! Pointer to the DIG1 Bank of the last event
   DWORD fDIG1BankLength;   //! Length  of the DIG1 Bank
   DWORD fLastDIG1BankLength;   //! Length  of the DIG1 Bank of the last event
   Bool_t  fDIG1BankExists;   //! Exist Flags of the DIG1 Bank
   Bool_t  fLastDIG1BankExists;   //! Exist Flags of the DIG1 Bank of the last event
   Bool_t  fDIG1BankActive;   //! Active Flags of the DIG1 Bank
   WORD*  fCAM0BankPointer;  //! Pointer to the CAM0 Bank
   WORD*  fLastCAM0BankPointer;  //! Pointer to the CAM0 Bank of the last event
   DWORD fCAM0BankLength;   //! Length  of the CAM0 Bank
   DWORD fLastCAM0BankLength;   //! Length  of the CAM0 Bank of the last event
   Bool_t  fCAM0BankExists;   //! Exist Flags of the CAM0 Bank
   Bool_t  fLastCAM0BankExists;   //! Exist Flags of the CAM0 Bank of the last event
   Bool_t  fCAM0BankActive;   //! Active Flags of the CAM0 Bank
   Bool_t  fDAQEventActive;    //! Active Flags of the DAQ Bank

   // Hot Links
   ReadDataHotLinks fReadDataHotLinks;               //! ReadData Hot Links
   WriteDataHotLinks fWriteDataHotLinks;               //! WriteData Hot Links
   GSPHotLinks fGSPHotLinks;               //! GSP Hot Links
   EventDisplayHotLinks fEventDisplayHotLinks;               //! EventDisplay Hot Links
   FeaturesHotLinks fFeaturesHotLinks;               //! Features Hot Links


private:
   CYGMidasDAQ(const CYGMidasDAQ &c); // not implemented
   CYGMidasDAQ &operator=(const CYGMidasDAQ &c); // not implemented

public:
   CYGMidasDAQ();
   virtual ~CYGMidasDAQ() {}
   // Bank Methodes
   void InitMidasBanks();
   DWORD  GetTDC0BankAt(Int_t indx) const;
   DWORD* GetTDC0BankPointer() const;
   DWORD  GetLastTDC0BankAt(Int_t indx) const;
   DWORD* GetLastTDC0BankPointer() const;
   Int_t GetTDC0BankEntries() const;
   Int_t GetLastTDC0BankEntries() const;
   void InitTDC0Bank();
   bool isTDC0BankActive() const { return fTDC0BankActive; }
   void SetTDC0BankActive(bool flag) { fTDC0BankActive = flag; }
   float  GetDIG0BankAt(Int_t indx) const;
   float* GetDIG0BankPointer() const;
   float  GetLastDIG0BankAt(Int_t indx) const;
   float* GetLastDIG0BankPointer() const;
   Int_t GetDIG0BankEntries() const;
   Int_t GetLastDIG0BankEntries() const;
   void InitDIG0Bank();
   bool isDIG0BankActive() const { return fDIG0BankActive; }
   void SetDIG0BankActive(bool flag) { fDIG0BankActive = flag; }
   float  GetDIG1BankAt(Int_t indx) const;
   float* GetDIG1BankPointer() const;
   float  GetLastDIG1BankAt(Int_t indx) const;
   float* GetLastDIG1BankPointer() const;
   Int_t GetDIG1BankEntries() const;
   Int_t GetLastDIG1BankEntries() const;
   void InitDIG1Bank();
   bool isDIG1BankActive() const { return fDIG1BankActive; }
   void SetDIG1BankActive(bool flag) { fDIG1BankActive = flag; }
   WORD  GetCAM0BankAt(Int_t indx) const;
   WORD* GetCAM0BankPointer() const;
   WORD  GetLastCAM0BankAt(Int_t indx) const;
   WORD* GetLastCAM0BankPointer() const;
   Int_t GetCAM0BankEntries() const;
   Int_t GetLastCAM0BankEntries() const;
   void InitCAM0Bank();
   bool isCAM0BankActive() const { return fCAM0BankActive; }
   void SetCAM0BankActive(bool flag) { fCAM0BankActive = flag; }
   bool isDAQEventActive() const { return fDAQEventActive; }
   void SetDAQEventActive(bool flag) { fDAQEventActive = flag; }

   // Hot Links
   ReadDataHotLinks* GetReadDataHotLinks() { return &fReadDataHotLinks; }
   WriteDataHotLinks* GetWriteDataHotLinks() { return &fWriteDataHotLinks; }
   GSPHotLinks* GetGSPHotLinks() { return &fGSPHotLinks; }
   EventDisplayHotLinks* GetEventDisplayHotLinks() { return &fEventDisplayHotLinks; }
   FeaturesHotLinks* GetFeaturesHotLinks() { return &fFeaturesHotLinks; }

protected:
   Bool_t  InitODB();
   Bool_t  InitHeader();
   Bool_t  IsActiveEventID(Int_t id) const ;
   void *ByteSwapStruct( char* aName, void* pData ) const;

   ClassDef(CYGMidasDAQ, 0)
};

#endif   // CYGMidasDAQ_H
