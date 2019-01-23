# THistSvc

The latest changes (2018-03-08) of the THistSvc provide two improvements:
 1. Threadfriendly management of TObjetcs (histograms, graphs, trees)
 1. Better memory management through communicating ownership with `std::unique_ptr`s in the interface

## Parallel THistSvc
Parallel usage of the THistSvc is possible through either:
 1. Use `regHist()` to register a clone for each involved thread that can be merged in the end. (e.g. during `finalize` of an Algorithm/Service ...)
 1. Use `regSharedHist()` to register a single instance for all threads that is mutexed and accessed through a `LockedHandle<>`

Both approaches have their use cases, but for moderate histogram usage during a normal job of some kind, sharing a single instance is probably best.

In future ROOT versions (ROOT 7?) it might be possible to introduce further methods of parallelized histograms.
Histograms with atomic bins looks very promising.

## About ownership
You should register your TObejects to the THistSvc through moving `std::unique_ptr`s into the `reg*` method.
The THistSvc manages these objects and is also responsible for writing these to ROOT files etc.

If your usage looked like this before:
```cpp
TH1D* hist = new TH1D(...);
regHist( "name", hist );
hist->Fill(...);
```
you should replace it with:
```cpp
std::unique_ptr<TH1D> hist = std::make_unique<TH1D>(...);
TH1D* histptr = hist.get();
regHist( "name", std::move(hist) );
histptr->Fill(...);
```
or even
```cpp
std::unique_ptr<TH1D> hist = std::make_unique<TH1D>(...);
regHist( "name", std::move(hist) );
TH1D* histptr = getHist( "name" );
histptr->Fill(...);
```

This way it is clear that the THistSvc takes ownership of these objects.
Working on the returned raw pointers is no issue as long as the THistSvc is still present (until `finalize` is called) and as long as the user does not do anything malicious with it (i.e. deleting it).
