#--------------------------------------------------------------
# PartPropSvc Service options
#--------------------------------------------------------------
theApp.Dlls += [ "PartPropSvc" ]
theApp.ExtSvc += ["PartPropSvc"]
PartPropSvc = Service( "PartPropSvc" )
PartPropSvc.InputType = "PDG"
PartPropSvc.InputFile = ["PDGTABLE.MeV"]
#--------------------------------------------------------------
