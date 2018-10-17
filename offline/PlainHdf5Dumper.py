#!/usr/bin/env python

import os,h5py
import ROOT
ROOT.gROOT.SetBatch(True)

class MidasToPlainHdf5Converter:
    def __init__(self,rfile,options):
        self.inputfile = rfile
        self.run = ((rfile.split('/')[-1]).split('.')[0]).split('DataTree')[-1]
        self.outputdir = options.outdir

    def th2h5(self,th2,h5file):
        nx,ny = (th2.GetNbinsX(),th2.GetNbinsY())
        dset = h5file.create_dataset(th2.GetName(), (nx,ny), dtype='i')
        for xb in xrange(1,nx+1):
            for yb in xrange(1,ny+1):
                print "xb,yb=",xb,",",yb," bc = ",int(th2.GetBinContent(xb,yb))
                dset[xb-1,yb-1] = int(th2.GetBinContent(xb,yb))
        
    def dumpAllPicsRAW(self):
        ROOT.gStyle.SetOptStat(0)
        f = ROOT.TFile.Open(self.inputfile)
        c1 = ROOT.TCanvas("c1","",600,600)

        if not os.path.isdir(self.outputdir):
            "Creating results directory ",self.outputdir
            os.system('mkdir {od}'.format(od=self.outputdir))

        of = h5py.File('{od}/histograms_Run{run}.hdf5'.format(od=self.outputdir,run=self.run), "w")
        print '===> Processing run # {run}; will put results in {of}'.format(run=self.run,of=of)
        
        for ie,event in enumerate(f.DataTree):
            if ie==options.maxEntries: break
            print "Processing event #{ie}...".format(ie=ie)

            postfix = 'run{run}_ev{ev}'.format(run=self.run,ev=ie)
            # get the TH2F of the camera picture
            pic = event.CamPicture
            pic.SetName('pic_{pfx}'.format(pfx=postfix))
            # convert the TH2 into a hdf5 and write to file
            self.th2h5(pic,of)
            
        of.close()
        f.Close()

if __name__ == '__main__':

    from optparse import OptionParser
    parser = OptionParser(usage='%prog DataTree1.root,...,DataTreeN.root [opts] ')
    parser.add_option('-o',   '--outdir', dest='outdir', type="string", default='./', help='directory where storing the ROOT files with the dump the TH1 and TH2');
    parser.add_option(        '--max-entries', dest='maxEntries', type='int', default=-1, help='Maximum number of events to process (default is all)');
    (options, args) = parser.parse_args()

    print "args = ",args
    for infile in args:
        print "infile = ",infile
        conv = MidasToPlainHdf5Converter(infile,options)
        conv.dumpAllPicsRAW()
