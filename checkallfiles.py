from ROOT import TFile, TTree
import pickle

rootpath = 'heppy.analyzers.examples.zh_had.TreeProducer.TreeProducer_1/tree.root' # output root file
picklepath = 'heppy.analyzers.examples.zh_had.Selection.Selection_1/cut_flow.pck' # pickle file containing the cut flow

def compare_event_count(path, verbose=False):
    rootfile = TFile('/'.join([path, rootpath]))
    tree = TTree()
    rootfile.GetObject("events", tree)
    n_events = tree.GetEntries()

    cutflow = pickle.load(open('/'.join([path, picklepath])))
    allevents = cutflow['All events'][1]

    if verbose or not n_events == allevents:
        print path
        print 'cut flow count:  ', allevents
        print 'ROOT file count: ', n_events
        print 'difference       ', abs(allevents-n_events)


if __name__ == '__main__':
    
    import argparse

    parser = argparse.ArgumentParser(
        description='''
if a 'LookupError: no such object, "events"' or 'IOError: no such file or directory' is encountered, you most likely have to edit the script and make sure that the variables rootpath and picklepath point to existing files'''
        )
    parser.add_argument('paths',
                        metavar='path',
                        nargs='+',
                        type=str,
                        help='Path (or list of paths) to a folder with analysis results from heppy'
                        )
    parser.add_argument('-v', '--verbose', action='store_true')

    args = parser.parse_args()
    for p in args.paths:
        compare_event_count(p, args.verbose)

    
