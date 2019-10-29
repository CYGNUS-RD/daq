#!/usr/bin/env python

import os
import ROOT
ROOT.gROOT.SetBatch(True)

class MidasToPlainROOTConverter:
    def __init__(self,rfile,options):
        self.inputfile = rfile
        self.run = ((rfile.split('/')[-1]).split('.')[0]).split('DataTree')[-1]
        self.outputdir = options.outdir

    def dumpAllPicsRAW(self,options):
        ROOT.gStyle.SetOptStat(0)
        f = ROOT.TFile.Open(self.inputfile)
        c1 = ROOT.TCanvas("c1","",600,600)

        if not os.path.isdir(self.outputdir):
            "Creating results directory ",self.outputdir
            os.system('mkdir {od}'.format(od=self.outputdir))

        of = ROOT.TFile.Open('{od}/histograms_Run{run}.root'.format(od=self.outputdir,run=self.run),'recreate')
        print '===> Processing run # {run}; will put results in {of}'.format(run=self.run,of=of)

        if options.plotDir:
            outdir = '{od}/run{run}'.format(od=options.plotDir,run=self.run)
            print "Saving pictures in ",outdir
            c1 = ROOT.TCanvas("c1","",600,600)
            ROOT.gStyle.SetPalette(ROOT.kTemperatureMap)
            if not os.path.isdir(outdir):
                os.system('mkdir '+outdir)
                os.system("cp index.php {od}".format(od=outdir))
                
        for ie,event in enumerate(f.DataTree):
            if ie==options.maxEntries: break
            print "Processing event #{ie}...".format(ie=ie)

            postfix = 'run{run}_ev{ev}'.format(run=self.run,ev=ie)
            # get the TH2F of the camera picture
            pic = event.CamPicture
            pic.SetName('pic_{pfx}'.format(pfx=postfix))
            pic.SetTitle('Camera, {pfx}'.format(pfx=postfix))
            pic.GetZaxis().SetRangeUser(98,120)
            if options.plotDir:
                pic.Draw('colz0')
                for ext in ['png']: #,'pdf']:
                    c1.SaveAs('{od}/{name}.{ext}'.format(od=outdir,name=pic.GetName(),ext=ext))
            
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
    parser.add_option('-p',   '--pdir', dest='plotDir', type='string', default=None, help='If given, the 2D pictures are drawn and saved in this dir');
    (options, args) = parser.parse_args()

    print "args = ",args
    for infile in args:
        print "infile = ",infile
        conv = MidasToPlainROOTConverter(infile,options)
        conv.dumpAllPicsRAW(options)
