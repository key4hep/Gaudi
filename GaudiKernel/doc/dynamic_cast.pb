This file is intended to describe the problems we had with dynamic_casts when
migrating from gcc 2.95.2 to gcc 3.2 under linux. All investigations and
solutions we tried are explain here so that we don't loose the knowledge.


The Situation
-------------

Here is a typical example of a Gaudi application :
  We have three main components : GaudiMain.exe, libGaudiSvc.so, libHbookCnv.so.
The first one is the executable, it loads dynamically (using dlopen) the two
others.
  The two libraries are sharing some abtract interface IHistogram by including
the same IHistogram.h file coming from the AIDA distribution. On top of that,
a concrete type called H1D is defined in libGaudiSvc that implements IHistogram
and inherits from DataObject. The key point here is that the main program
GaudiMain (and the package GaudiKernel which is linked to it) only know
about the DataObject type but have no clue what an IHistogram is.

  The way the program runs is that GaudiMain first loads libGaudiSvc, calls a
function on it that creates a H1D object and returns it as a DataObject* to
the Gaudi transient store. Later, GaudiMain gives this object to libHbookCnv
so that it is written to a file. Since libHbookCnv wants to use a IHistogram but
is givena IHistogram*, it issues a dynamic_cast on the DataObject* to
IHistogram*.


The problem
-----------

  All this was working fine on gcc 2.95 but it becomes more complicated with
3.2. The key point here is that the essence of type equality changed from one
version to the other. 2.95 assumes two types to be equal if they have the same
name. In 3.2, this is not true anymore. Two types are equal if and only if
the pointers to their typeid names are equal.
  There is an immediate advantage in using the second version of typeid
equality : imagine that two types are defined in two different libraries with
the same name but different purposes; let say IHistogram is defined once in
AIDA and once in a gnuplot C library. In 2.95 you were able to dynamic_cast
one into the other creating a huge mess in memory. Now it won't work.
  Now the problem is that it will not work better if the two types actually
are the same one. In our case, the IHistogram type (which in defined in a .h,
remember it) is defined (by inclusion) in both libGaudiSvc and libHbookCnv.
The two definitions are identical but a dynamic_cast from one to the other
will fail, breaking the writing of histograms into hbook files.


Some prerequisite : weak versus strong symbols in libraries
-----------------------------------------------------------

  Under some platforms (linux among others), the symbols can appear under two
forms in a library : as weak or as strong. The difference will show up in the
way they are used at link time (be it static or dynamic linking).
  In the case of string symbols, the linker expects the symbol to be unique
among the linked libraries and to exist. This unique symbol will then be used
by all libraries needing it.
  On the contrary, weak symbols can very well exist many or zero times. This
will not cause any problem. If no symbol is found, a null one is provided. In
case many symbols are found, the first one is taken and used by everybody.
This mechanism allows to easily overwrite some entry points in a library by
providing another implementation for the same symbol. As an example, imagine
you want to change the behavior of a function called foo (and declared as a
weak symbol) from the library libfoo.so that you don't own. Just create a
libmyfoo.so library, reimplementing foo and link it with the original libfoo.
Now replace the usage of libfoo by libmyfoo and that's it.
  Let me finish this prerequisite by remembering two points :
    - every type_info symbols are weak
    - in case one symbols has many definitions, one of them being strong, the
other ones weak, the string one is used.


First attempt : look at the gcc web site
----------------------------------------
  
  The gcc web site actually has an FAQ dealing with the problem (see
http://gcc.gnu.org/faq.html#dso). Here is the relevant part of it for us :
      "You must also make the external symbols in the loaded library
       available for subsequent libraries by providing the RTLD_GLOBAL
       flag to dlopen."
  The use of RTLD_GLOBAL here allows the symbols of the loaded library to be
globally seen by all other libraries. This has two different consequences :
the first is that all string symbols must be uniquely defined among all
libraries while the second is that the definition of a weak symbol that will
be used is unique among all libraries, and happens to be the first one found.
  This means that we go back to a situation that is worse than the 2.95 one :
we cannot even use AIDA and gnuplot at the same time now, because of symbol
clashes. The risk is that two IHistogram symbols clash and that one library
will use the IHistogram of the other leading to memory corruption.
  No way we use this


Second attempt : try to share a common definition of typeids
------------------------------------------------------------

  The idea is the following. We are in a situation where a type (IHistogram)
is defined in two places and we basically consider that it's two different
types, even if the definitions are identical. Let's try to share a unique
definition and we'll solve the problem.
  Let's may a short drawig to illustrate that :

                 GaudiMain                        GaudiMain
                 ---------                        ---------
                 /       \           =>            /     \
           GaudiSvc      HbookCnv           GaudiSvc     HbookCnv
           --------      --------           --------     --------
                                                   \     /
                                                     AIDA
                                                     ----

  What we propose is to add a new library, called libAIDA, where we uniquely
define IHistogram and that we link with every library sharing this IHitogram
type. In the case we also use gnuplot, the gnuplot library will also define
a IHistogram type and be linked to whatever library needs it.
  This is very good theory but it happens to be of poor use in real life.
Actually, our unique definition of IHistogram will be far from being unique
since the definitions in GaudiSvc and HbookCnv will remain. This is thus just
one more. Still we have an hope that it is the one used but this is not even
the case.
  The problem resides in the order in which libraries are loaded into memory.
Let's say GaudiMain loads first GaudiSvc and then HbookCnv. What will happen
in memory is the following :
  - firts put GaudiSvc into memory
  - then put AIDA into memory and deal with weak symbols. In case they are
already defined in GaudiSvc, reuse them. Thus, Aida will use GaudiSvc's
IHistogram.
  - then put HbookCnv into memory. since this one cannot see GaudiSvc nor
AIDA (yet), it uses it's own IHistogram definition
  - link AIDA to HbookCnv without reloading it, since it's there. No change
of symbols anymore in AIDA.

  The result is that GaudiSvc and HbookCnv still use their own Ihistogram
definitions ! It's still worth to mention the kind of code we used to define
the AIDA library :

   #include "AIDA/IHistogram.h"
   namespace {
     const std::type_info& IHistoTypeInfo = typeid(IHistogram);
   }


Third attempt : make the common definition a strong symbol
----------------------------------------------------------

  This would insure that the common definition is the only one used everywhere.
As a matter of fact, in point 2 of the loading process we described above,
GaudiSvc's weak symbol will be replaced by the AIDA strong symbol definition.
Same for HbookCnv in point 4.
  But how to make the symbol strong ? This is basically where we failed. It
seems there is no way to achieve this using regular C++. No way to express it
in C++ (even if "" exists to weaken a symbol), no tool to manipulate the
existing library this way (even if objcopy -W allows to weaken a symbol),
no way to give an hint at link time. Nothing.
  Still, there was a solution : using directly assembler code to define the
typeinfo symbol of IHistogram. The code looks like this :

    .globl _ZTS10IHistogram
    .globl _ZTI10IHistogram
    _ZTS10IHistogram:
         .string "IHistogram"
    _ZTI10IHistogram:
         .long _ZTVN10__cxxabiv117__class_type_infoE+8
         .long _ZTS10IHistogram

  Don't think that .globl makes the symbol global, it just makes it visible from
outside. One could also give an example of the code that you would write for a
more complicated case where the class inherits from other classes :

    .globl _ZTS12IHistogram1D
    .globl    _ZTI12IHistogram1D
    _ZTS12IHistogram1D:
        .string  "IHistogram1D"
    _ZTI12IHistogram1D:
        .long  _ZTVN10__cxxabiv121__vmi_class_type_infoE+8
        .long  _ZTS12IHistogram1D
        .long  8
        .long  1
        .long  _ZTI10IHistogram
        .long  -21501
        .zero  8
        .align 32

  Not really nice ! On top of that, such code has to be written for each class
that could be shared one day, basically for every class. No really manageable.
Actually, when you think about it, a solution where you have to list all the
classes you may share is not manageable, thus even using objcopy would not have
been a solution.
  But this proved to work nicely still.


Fourth attempt : redefine the type_info::operator==
---------------------------------------------------

  This idea came from Stefan (Roiser) during a Gaudi meeting. Since we can
overwrite weak symbols, why not to overwrite the type_info::operator== in order
to change back it's behavior to comparing type_info names and not pointers ?

  Very Nice idea indeed. So we added this to GaudiKernel :

     bool std::type_info::before (const type_info &arg) const {
       return strcmp (name (), arg.name ()) < 0;
     }
     bool std::type_info::operator== (const std::type_info& arg) const {
       return (&arg == this) || (__builtin_strcmp (name (), arg.name ()) == 0);
     }

  And recompiled everything. No improvement ! After many hours spend on gdb, the
reason was very clear : the initial definition of type_info::operator== in the
standard library is inlined. Thus it was copied wherever used in the standard
library (specifically in dynamic_cast) and our, not inlined definition was never
called. Too bad. End of the game...
  Is it ? What if we also overwrite the dynamic_cast function itself ?


The current solution : redefining dynamic_cast and type_info
------------------------------------------------------------

  Overwriting the dynamic_cast function of the standard library is really not
a easy task due to its complexity. However, libstdc++ is an open source project
and thus, we are allowed to directly cut and past their code !
  Even better, if you look carefully at the code, you see that the old, gcc 2.95
version of the type_info equality is still there. It is still used for platforms
without weak symbols. This means that we don't even have to modify anything in
the code, we just include it into GaudiKernel and let it think that we are
on a platform without weak symbols. Here is the code :

    #  undef __GXX_WEAK__
    #    include "../gcc/tinfo.cc"
    #  define __GXX_WEAK__ 1

  Just introducing these three lines into GaudiKernel solved all problems.
Actually, one has to mention that we also had to keep a copy of three files
coming from the libstdc++ distribution in GaudiKernel : tinfo.cc, tinfo.h and
typeinfo. This is because they are not present by default on every linux
installation since they (2 out of 3) belong to the sources of libstdc++.


Advantages and drawbacks of the solution
----------------------------------------

  The main and only important advantage is that all is working as before.
Exactly as before even, since we removed the difference of behavior for the
type_info comparisons. Another point is that the addition to the previous
code is really minimum (3 short lines) and easy to maintain since we include
unmodified files from the libstdc++ distribution.
  Still, these files will need to be updated when we go to newer releases of
libstdc++. On top of that, the inclusion needs to be included inside a ifdef
statement which selects the right version of linux.
  The main drawbacks of this solution are more coming from the "moral" point
of view. First, the solution simply consists in saying : "We don't want to
cope with the new C++ ABI, give us back the old one". It's easy today since
we are in a transition but what in 10 years ? Will we still have to include
a 10 years old version of dynamic_cast ? Will we have to rewrite it ourselves
in order to make it compatible ?
  Another problem is that the solution is less general than the one using
assembler. Actually, it just has the drawbacks of the old C++ ABI, which means
that in the case of AIDA and gnuplot defining the same IHistogram type, one can
still synamic_cast from one to the other definition, corrupting immediately the
memory.


A word about python scripting
-----------------------------

  It is worth to mention here that all this story would not have been so long
without the presence of python scripting inside Gaudi. Let's detail a bit why.

  The first point is that even if the problem of GaudiSvc and HbookCnv we
mentionned really existed in Gaudi, it was actually the only one known in the
whole LHCb code. Thus, a dirty solution was to make GaudiKernel include
AIDA/IHistogram.h in order to define IHistogram at the top level. This could
well have been the definite solution if we would not have tried the python
examples. Actually, these ones were all broken. This is due to the way we load
libraries when using python. Here is an overview of it :

                        python
                        ------
                           |
                      GaudiModule   GaudiSvc
                      -----------   --------
                              \       /
                             GaudiKernel
                             -----------

  Python is called first and loads GaudiModule. GaudiKernel is immediately
loaded because it's linked with GaudiModule and latter GaudiSvc is loaded,
which is also linked with GaudiKernel.
  One immediately sees on the diagram that GaudiModule and GaudiSvc will not
be able to share types which are fully defined in both (such as IProperty).
Thus, python types will never be able to be casted to C++ corresponding types.

  The second point is that the very first try, using RTLD_GLOBAL, even if not
acceptable from the theoretical point of view would have worked (and actually
worked) for Gaudi, as far as python is not included. This is due to the fact
that we actually don't have any clash in any symbol anywhere, as far as python
is not loaded. Actually, even the previous example, where Gaudi is loaded from
python worked. The problem is when we launch python from Gaudi, as in the
GPython example of the GaudiExamples. Then the structure is the following :

                      GaudiModule   GaudiSvc
                      -----------   --------
                              \      /
                             GaudiKernel
                             -----------
                                  |
                              libpython
                              ---------

  Here GaudiKernel is loaded first, followed by GaudiSvc. Then libpython is
loaded that will load in turn GaudiModule. The problem is that somehow,
libpython reuses one symbol already defined either in GaudiSvc or GaudiKernel.
This would never happend without RTLD_GLOBAL since it is GaudiKernel that could
reuse symbols of libpython and not the reverse. But with RTLD_GLOBAL, it's
possible and the reuse leads to problems.


Pfff... That's all this time.
Nice to have it written and solved !

Sebastien Ponce
