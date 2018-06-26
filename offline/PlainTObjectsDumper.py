#!/usr/bin/env python

import os
import ROOT
ROOT.gROOT.SetBatch(True)

class MidasToPlainROOTConverter:
    def __init__(self,rfile,options):
        self.inputfile = rfile
        self.run = ((rfile.split('/')[-1]).split('.')[0]).split('DataTree')[-1]
        self.outputdir = options.outdir

    def dumpAllPicsRAW(self):
        ROOT.gStyle.SetOptStat(0)
        f = ROOT.TFile.Open(self.inputfile)
        c1 = ROOT.TCanvas("c1","",600,600)

        if not os.path.isdir(self.outputdir):
            "Creating results directory ",self.outputdir
            os.system('mkdir {od}'.format(od=self.outputdir))

        of = ROOT.TFile.Open('{od}/histograms_Run{run}.root'.format(od=self.outputdir,run=self.run),'recreate')
        print '===> Processing run # {run}; will put results in {of}'.format(run=self.run,of=of)
        
        for ie,event in enumerate(f.DataTree):
            if ie==options.maxEntries: break
            print "Processing event #{ie}...".format(ie=ie)

            postfix = 'run{run}_ev{ev}'.format(run=self.run,ev=ie)
            # get the TH2F of the camera picture
            pic = event.CamPicture
            pic.SetName('pic_{pfx}'.format(pfx=postfix))
            pic.SetTitle('Camera, {pfx}'.format(pfx=postfix))
            pic.GetZaxis().SetRangeUser(95,130)

            # get the TGraph of the PMT waveform (only the used channel)
            wfm = event.PMT
            wfm.SetName('wfm_{pfx}'.format(pfx=postfix))
            wfm.SetTitle('PMT, {pfx}'.format(pfx=postfix))

            # write to the output ROOT file
            wfm.Write()
            pic.Write()
            
        of.Close()
        f.Close()

if __name__ == '__main__':

    from optparse import OptionParser
    parser = OptionParser(usage='%prog DataTree1.root,...,DataTreeN.root [opts] ')
    parser.add_option('-o',   '--outdir', dest='outdir', type="string", default='./', help='directory where storing the ROOT files with the dump the TH1 and TH2');
    parser.add_option(        '--max-entries', dest='maxEntries', type='int', default=-1, help='Maximum number of events to process (default is all)');

    (options, args) = parser.parse_args()

    for infile in args:
        conv = MidasToPlainROOTConverter(args[0],options)
        conv.dumpAllPicsRAW()
