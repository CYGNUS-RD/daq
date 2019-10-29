#!/usr/bin/env python

import os
import ROOT
import root_numpy as rtnp
ROOT.gROOT.SetBatch(True)

tf = ROOT.TFile("results/histograms_Run00936.root")
mypic = tf.Get("pic_run00936_ev0")
myarray = rtnp.hist2array(mypic)
print myarray

