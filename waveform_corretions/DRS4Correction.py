import PMTreco as rec
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from tqdm.notebook import tqdm
import os


def GetInfoForCorrections(runs):
    listwfs = []
    listSIC = []

    counter  = 0

    for r in runs:
        print('run = ', r)

        dgh0 = np.load('./tmp/DGH_{:05d}.npy'.format(r), allow_pickle = True)
        wfs  = np.load('./tmp/WFs_{:05d}.npy'.format(r), allow_pickle = True)

        Nch = 8
        Npics  = len(wfs)
        for ch in range(Nch):
            for i in range(min(Npics, 100)):
                print('Pics: {0:05d} over {1:d}'.format(i, Npics), end = '\r')
                Ntr = len(wfs[i])
                #print('{:05d}'.format(i), end = '\r')
                nevtmp = dgh0[i][4]
                stcstmp = dgh0[i][(7+32+nevtmp):(7+32+2*nevtmp)]
                for j in range(Ntr):
                    if(i == 0 and j == 0 and r == runs[0]):
                        listwfs.append(wfs[i][j][ch])
                        if ch == 0:
                            listSIC = stcstmp[j]
                    else:
                        listwfs[ch] = np.vstack([listwfs[ch], wfs[i][j][ch]])
                        if ch == 0:
                            listSIC = np.append(listSIC,stcstmp[j])

    L = listwfs[0].shape[0]

    for ch in range(Nch):
        for i in range(L):
            print('{0:05d} over {1:d}'.format(i, L), end = '\r')
            tmp = np.roll(listwfs[ch][i],  listSIC[i])
            listwfs[ch][i] = tmp
            
    return listwfs, listSIC


def CellCorrection(wf_in, Nch = 8):
    wf = np.copy(wf_in)
    Npics = len(wf[0])
    residuals_corr = []
    compute_tab = False
    if(os.path.exists('./table_cell.npy')):
        table_cell = np.load('./table_cell.npy')
    else:
        table_cell = []
        compute_tab = True
        
    for ch in range(Nch):
        residuals_corr.append(np.mean(wf[ch], axis = 0))
        if(compute_tab):
            table_cell.append(residuals_corr[ch].astype(int)-np.mean(residuals_corr[ch]).astype(int))
        for i in range(Npics):
            wf[ch][i] = wf_in[ch][i] - table_cell[ch]
            
    if(compute_tab):
        print("Saving recomputed tables...")
        table_cell = np.array(table_cell)
        np.save('./table_cell.npy', table_cell)
    return wf



def nSampleCorrection(wf_in, sic, Nch = 8):
    wf = np.copy(wf_in)
    Npics = len(wf[0])
    
    compute_tab = False
    if(os.path.exists('./table_nsample.npy')):
        table_nsample = np.load('./table_nsample.npy')
    else:
        table_nsample = []
        compute_tab = True
    
    for ch in range(Nch):
        for i in range(Npics):
            wf[ch][i] = np.roll(wf[ch][i], -sic[i])
            
    for ch in range(Nch):
        if(compute_tab):
            tmp_mean   = np.mean(wf[ch], axis = 0).astype(int)
            tmp_middle = np.mean(np.mean(wf[ch], axis = 0).astype(int))
            table_nsample.append(tmp_mean-tmp_middle)
        for i in range(Npics):
            wf[ch][i] = wf[ch][i] - table_nsample[ch]
    
    if(compute_tab):
        print("Saving recomputed tables...")
        table_nsample = np.array(table_nsample)
        np.save('./table_nsample.npy', table_nsample)
    
    return wf


def PeakCorrection(wf_in, Nch = 8):
    wf = np.copy(wf_in)
    Npics = wf[0].shape[0]
    avgs = []
    for ch in range(Nch):
        avgs.append(np.mean(wf[ch]))
    print('\n')
    for k in range(Npics):
        print(k, end = '\r')
        for i in range(1, 1024):
            offset  = 0
            offset_plus = 0
            for ch in range(Nch): #for over the channels
                if i ==1: ## non va bene, servono tutti e 8
                    if (wf[ch][k][2] - wf[ch][k][1])>30:
                        offset += 1
                    else:
                        if (wf[ch][k][3]-wf[ch][k][1])>30 and (wf[ch][k][3]-wf[ch][k][2])>30:
                            offset += 1
                else:
                    if i == (1024-1) and (wf[ch][k][1024-2] - wf[ch][k][1024-1])>30:
                        offset+=1
                    else:
                        if (wf[ch][k][i-1]-wf[ch][k][i])>30:
                            if (wf[ch][k][i+1] - wf[ch][k][i])>30:
                                offset += 1
                            elif (i+2)<1024-2:
                                if (wf[ch][k][i+2] - wf[ch][k][i])>30 and (wf[ch][k][i+1] - wf[ch][k][i])<5:
                                    offset += 1
                            else:
                                if i == (1024-2) or (wf[ch][k][i+2]-wf[ch][k][i])>30:
                                    offset += 1
                                    
                if i < (1024-6) and (avgs[ch] - wf[ch][k][i])<-30 and \
                   (avgs[ch] - wf[ch][k][i+1])<-30 and \
                   (avgs[ch] - wf[ch][k][i+2])<-30 and \
                   (avgs[ch] - wf[ch][k][i+3])<-30 and \
                   (avgs[ch] - wf[ch][k][i+4])<-30 and \
                   (avgs[ch] - wf[ch][k][i+5])<-30:
                        offset_plus += 1
                        #print(offset_plus)
            
            #if(offset_plus!= 0):
                #print(k, "  ", i, "  ", offset_plus)
            if offset == 8:
                #print(i)
                for ch in range(Nch):
                    if i ==1:
                        if (wf[ch][k][2] - wf[ch][k][1])>30:
                            wf[ch][k][0] = wf[ch][k][2]
                            wf[ch][k][1] = wf[ch][k][2]
                        else:
                            wf[ch][k][0] = wf[ch][k][3]
                            wf[ch][k][1] = wf[ch][k][3]
                            wf[ch][k][2] = wf[ch][k][3]
                    else:
                        if i == (1024-1):
                            wf[ch][k][1024-1]=wf[ch][k][1024-2]
                        else:
                            if (wf[ch][k][i+1]-wf[ch][k][i])>30:
                                if (wf[ch][k][i+1] - wf[ch][k][i])>30:
                                    wf[ch][k][i] =  int((wf[ch][k][i+1]+ wf[ch][k][i-1])/2)
                                elif (i+2)<1024-2:
                                    if (wf[ch][k][i+2] - wf[ch][k][i])>30 and (wf[ch][k][i+1] - wf[ch][k][i])<5:
                                        wf[ch][k][i] =  int((wf[ch][k][i+2]+ wf[ch][k][i-1])/2)
                                        wf[ch][k][i+1] =  int((wf[ch][k][i+2]+ wf[ch][k][i-1])/2)
                            else:
                                if i == (1024-2):
                                    wf[ch][k][1024-2] = wf[ch][k][1024-3]
                                    wf[ch][k][1024-2] = wf[ch][k][1023-3]
                                else:
                                    wf[ch][k][i]   = int((wf[ch][k][i+2]+wf[ch][k][i-1])/2)
                                    wf[ch][k][i+1] = int((wf[ch][k][i+2]+wf[ch][k][i-1])/2)
            
            if offset_plus==8:
                for ch in range(Nch):
                    for m in range(6):
                        wf[ch][k][i+m] = avgs[ch]
    
    print('\n')                     
    return wf



