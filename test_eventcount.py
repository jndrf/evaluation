import unittest
import tempfile
import copy
import os
import shutil

import heppy.framework.context as context

if context.name == 'fcc':

    from heppy.test.analysis_ee_ZH_had_cfg import config
    from heppy.test.plot_ee_ZH import plot
    from heppy.framework.looper import Looper
    from ROOT import TFile, TTree

    import logging
    logging.getLogger().setLevel(logging.ERROR)

    import heppy.statistics.rrandom as random

    def test_sorted(ptcs):
        from heppy.configuration import Collider
        keyname = 'pt'
        if Collider.BEAMS == 'ee':
            keyname = 'e'
        pt_or_e = getattr(ptcs[0].__class__, keyname)
        values = [pt_or_e(ptc) for ptc in ptcs]
        return values == sorted(values, reverse=True)


    class TestAnalysis_ee_ZH_had(unittest.TestCase):

        def setUp(self):
            random.seed(0xdeadbeef)
            self.outdir = tempfile.mkdtemp()
            import logging
            logging.disable(logging.CRITICAL)

        def tearDown(self):
            shutil.rmtree(self.outdir)
            logging.disable(logging.NOTSET)

        def test_event_counting(self):
            '''
            check if the event count given by the cut flow tool matches
            the count in the ROOT file
            '''
            from heppy.papas.detectors.CMS import cms
            for s in config.sequence:
                if hasattr( s,'detector'):
                    s.detector = cms
            # import pdb; pdb.set_trace()
            fname = '/afs/cern.ch/user/j/jneundor/work/public/FCCSamples/data/plain/ee_ZH_any_0.root'#'/'.join([os.environ['HEPPY'],
#                                      'test/data/ee_ZH_Zmumu_Hbb.root'])
            config.components[0].files = [fname]
            looper = Looper( self.outdir, config,
                                          nPrint=0 )
            looper.loop()
            looper.write()
            rootfile = TFile('/'.join([self.outdir,
                                'heppy.analyzers.examples.zh_had.TreeProducer.TreeProducer_1/tree.root'])
                                  )
            tree = TTree()
            rootfile.GetObject("events", tree)
            n_events = tree.GetEntries()

            # load pickle file of Selection
            import pickle
            cutflow = pickle.load(open('/'.join([self.outdir, 'heppy.analyzers.examples.zh_had.Selection.Selection_1/cut_flow.pck'])))
            allevents = cutflow['All events'][1]
            self.assertEqual(n_events, allevents)
            

        


if __name__ == '__main__':

    unittest.main()
