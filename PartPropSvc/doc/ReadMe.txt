-----------------------------------------------------------------------------
Usage:
-----------------------------------------------------------------------------

JobOptions:
   (string) "InputType": type of input format. One of:
                         "PDG", "Herwig", "Pythia", "IsaJet"
                         default: "PDG"

   (vector<string>) "InputFile": name(s) of input files


-----------------------------------------------------------------------------
Accessors:
-----------------------------------------------------------------------------

   HepPDT::ParticleDataTable* PDT():  returns the particle data table
                                      generated from input files read
                                      in.

  



