## -*- python -*-
## automatically generated from a hscript
## do NOT edit.

## waf imports
import waflib.Logs as msg

PACKAGE = {
    "name":    "GaudiPolicy",
    "authors": [],

    "version":  "v14r0",
}

### ---------------------------------------------------------------------------
def pkg_deps(ctx):

    ## public dependencies
    ctx.use_pkg("LCG_Settings", version="*", public=True)
    ctx.use_pkg("LCG_Interfaces/Python", version="*", public=True)
    ctx.use_pkg("LCG_Interfaces/tcmalloc", version="*", public=True)

    ## no private dependencies
    ## runtime dependencies
    ctx.use_pkg("LCG_Interfaces/Python", version="*", runtime=True)
    ctx.use_pkg("LCG_Interfaces/tcmalloc", version="*", runtime=True)
    return # pkg_deps


### ---------------------------------------------------------------------------
def options(ctx):

    return # options


### ---------------------------------------------------------------------------
def configure(ctx):

    ### export
    ctx.hwaf_export_module("waftools/gaudi-policy.py")

    macro = ctx.hwaf_declare_macro

    ## FIXME
    ctx.hwaf_macro_append('CXXFLAGS', '-Wno-deprecated')
    macro('GENCONF_CONFIGURABLE_MODULE', 'GaudiKernel.Proxy')
    ##
    ctx.hwaf_declare_runtime_env('JOBOPTSEARCHPATH')
    ctx.hwaf_declare_runtime_env('DATAPATH')
    ctx.hwaf_declare_runtime_env('XMLPATH')

    ## FIXME
    ctx.hwaf_macro_append("DEFINES", (
        {"target-linux": ["linux", "f2cFortran", "_GNU_SOURCE", "unix"]},
        {"target-mac106": ["f2cFortran", "_GNU_SOURCE", "unix"]},
    ))

    ## FIXME
    ctx.hwaf_macro_append("DEFINES", (
      {"default": "GAUDI_V20_COMPAT"},
      {"GAUDI_V21": ""},
      {"GAUDI_V22": "GAUDI_V22_API"},
    ))
    ctx.hwaf_macro_append("DEFINES", (
      {"default": ""},
      {"ATLAS": "ATLAS_GAUDI_V21"},
    ))

    ## FIXME:
    if 1:
        return

    #### tag &{x86_64-slc5-gcc43-test [x86_64-slc5-gcc43-opt target-test]}
    ctx.hwaf_declare_tag(
        "x86_64-slc5-gcc43-test",
        content=["x86_64-slc5-gcc43-opt", "target-test"]
    )
    #### tag &{x86_64-slc5-gcc46-test [x86_64-slc5-gcc46-opt target-test]}
    ctx.hwaf_declare_tag(
        "x86_64-slc5-gcc46-test",
        content=["x86_64-slc5-gcc46-opt", "target-test"]
    )
    #### tag &{x86_64-slc6-gcc46-test [x86_64-slc6-gcc46-opt target-test]}
    ctx.hwaf_declare_tag(
        "x86_64-slc6-gcc46-test",
        content=["x86_64-slc6-gcc46-opt", "target-test"]
    )
    #### macro &{{LCG_platform [{default [${LCG_platform}]} {target-winxp [${LCG_platform}]} {LOCAL&target-dbg [${LCG_system}]} {LOCAL&target-cov [${LCG_system}]}]}}
    macro("LCG_platform", (
      {"default": "${LCG_platform}"},
      {"target-winxp": "${LCG_platform}"},
      {("LOCAL", "target-dbg"): "${LCG_system}"},
      {("LOCAL", "target-cov"): "${LCG_system}"},
    ))
    ##### **** statement *hlib.PatternStmt (&{include_none -global include_path none})
    ##### **** statement *hlib.PatternStmt (&{packageDir -global macro <package>Dir ${<PACKAGE>ROOT}/${BINDIR}})
    ##### **** statement *hlib.PatternStmt (&{install_more_includes document install_more_includes install_more_includes more=<more> offset=<offset> ; macro_prepend includes  <project>_without_installarea $(ppcmd)"$(<package>_root) ; private ; macro_prepend includes $(ppcmd)"$(<package>_root) <project>_without_installarea  ; end_private})
    ##### **** statement *hlib.MakeFragmentStmt (&{install_more_includes})
    #### macro &{{copyInclude [{default [cp -Rup]} {host-winxp [xcopy /S/Q/Y/I/D]} {host-darwin [cp -Rp]}]}}
    macro("copyInclude", (
      {"default": ["cp", "-Rup"]},
      {"host-winxp": ["xcopy", "/S/Q/Y/I/D"]},
      {"host-darwin": ["cp", "-Rp"]},
    ))
    #### macro &{{ar [{default [${ar}]} {target-darwin [libtool -o]}]}}
    macro("ar", (
      {"default": "${ar}"},
      {"target-darwin": ["libtool", "-o"]},
    ))
    #### macro &{{ranlib [{default [ranlib]} {target-darwin [touch]}]}}
    macro("ranlib", (
      {"default": "ranlib"},
      {"target-darwin": "touch"},
    ))
    ##### **** statement *hlib.PatternStmt (&{Obj2Doth document obj2doth <package>Obj2Doth ../xml/*.xml})
    ##### **** statement *hlib.PatternStmt (&{packageOpts set <PACKAGE>OPTS $(<package>_root)/options target-winxp $(<package>_root)\options})
    ##### **** statement *hlib.PatternStmt (&{RuleChecker document rule_checker <package>Chk -group=RuleChecker ../src/*.cpp ../src/Lib/*.cpp ../src/component/*.cpp})
    ##### **** statement *hlib.PatternStmt (&{librarySh set <library>Shr  <project>_without_installarea&target-unix $(<package>Dir)/lib<library> <project>_without_installarea&target-winxp $(<package>Dir)\<library>})
    ##### **** statement *hlib.PatternStmt (&{library_stamps macro <package>_stamps ${<PACKAGE>ROOT}/${BINDIR}/<library>.stamp})
    ##### **** statement *hlib.PatternStmt (&{library_shlibflags macro <library>_shlibflags $(libraryshr_linkopts)})
    ##### **** statement *hlib.PatternStmt (&{library_Cshlibflags private ; macro <library>_shlibflags $(componentshr_linkopts) $(cmt_installarea_linkopts) $(<library>_use_linkopts) ; end_private})
    ##### **** statement *hlib.PatternStmt (&{library_Llinkopts macro_append <package>_linkopts  <project>_with_installarea&target-unix -l<library> <project>_with_installarea&target-winxp <library>.lib <project>_without_installarea&target-unix -L$(<package>Dir) -l<library> <project>_without_installarea&target-winxp $(<package>Dir)/<library>.lib })
    ##### **** statement *hlib.PatternStmt (&{library_Lshlibflags private ; macro <library>_shlibflags $(libraryshr_linkopts) $(cmt_installarea_linkopts) $(<library>_use_linkopts) ; macro_append <library>_shlibflags  target-linux -Wl,-soname=$(library_prefix)<library>.$(shlibsuffix) ; macro_remove <library>_use_linkopts $(<package>_linkopts) ; end_private})
    ##### **** statement *hlib.PatternStmt (&{library_Softlinks macro_append <package>_libraries  <project>_without_installarea&target-unix <library> target-winxp})
    ##### **** statement *hlib.PatternStmt (&{library_path path_remove PATH  target-winxp \<package>" ; path_prepend PATH  <project>_without_installarea&target-winxp ${<package>_root}\${<package>_tag} ; path_remove DYLD_LIBRARY_PATH  target-darwin /<package>/ ; path_append DYLD_LIBRARY_PATH  <project>_without_installarea&target-darwin ${<package>_root}/${<package>_tag} ; apply_pattern library_Softlinks library="<library>})
    ##### **** statement *hlib.PatternStmt (&{component_library apply_pattern libraryShr library="<library> ; apply_pattern library_Cshlibflags library="<library> macro <library>_dependencies $(<package>_linker_library) apply_pattern generate_rootmap library=<library> group=<group> ; apply_pattern generate_configurables library=<library> group=<group>})
    ##### **** statement *hlib.PatternStmt (&{linker_library apply_pattern library_path library="<library> ; apply_pattern library_Llinkopts library="<library> ; apply_pattern library_Lshlibflags library="<library> ; apply_pattern library_stamps library="<library> ; macro <package>_linker_library <library> ; private ; macro_append lib_<library>_pp_cppflags -DGAUDI_LINKER_LIBRARY ; macro <library>exportsymb -export_all_symbols target-winxp -export_all_symbols GAUDI_V21  GAUDI_V22  G21_HIDE_SYMBOLS "; macro <library>installlib yes ; end_private})
    ##### **** statement *hlib.MakeFragmentStmt (&{data_installer})
    ##### **** statement *hlib.PatternStmt (&{declare_joboptions macro_append <package>_joboptions -s=../share <files> ; document data_installer install_<name>joboptions prefix=$(shared_install_subdir)/jobOptions $(<package>_joboptions) ; macro_remove constituents  target-winxp install_<name>joboptions})
    ##### **** statement *hlib.PatternStmt (&{declare_scripts macro_append <package>_scripts -s=$(<PACKAGE>ROOT)/share <files> ; document installer install_<name>scripts $(<package>_scripts) install_dir=$(CMTINSTALLAREA)/share/bin install_command="'$(symlinknew)'})
    ##### **** statement *hlib.PatternStmt (&{declare_python_modules macro_append <package>_python_modules -s=../python <files> ; document data_installer install_<name>python_modules prefix=python $(<package>_python_modules) ; macro_remove constituents  target-winxp install_<name>python_modules})
    ##### **** statement *hlib.PatternStmt (&{install_python_modules macro <package>_install_python  <project>_without_installarea&target-unix $(<package>_root)/python <project>_without_installarea&target-winxp $(<package>_root)\python ; path_remove PYTHONPATH ${<package>_install_python} ; path_prepend PYTHONPATH ${<package>_install_python} ; document install_python <package>_python ; apply_pattern genconfuser ; apply_pattern install_python_init ; macro_append <package>_python_init_dependencies <package>_python ;})
    ##### **** statement *hlib.PatternStmt (&{install_custom_python_modules macro <package>_install_python  <project>_without_installarea&target-unix $(<package>_root)/<source> <project>_without_installarea&target-winxp $(<package>_root)\<source> ; path_remove PYTHONPATH ${<package>_install_python} ; path_prepend PYTHONPATH ${<package>_install_python} ; document install_python <package>_<source>_<offset>_python source=<source> offset=<offset> ; apply_pattern install_python_init ; macro_append <package>_python_init_dependencies <package>_<source>_<offset>_python ;})
    ##### **** statement *hlib.MakeFragmentStmt (&{install_python})
    ##### **** statement *hlib.PatternStmt (&{install_python_init document python_init <group><package>_python_init -group=<group> package=<group><package> ; apply_pattern zip_python_modules package=<package> group=<group> ; macro_append zip_<group><package>_python_modules_dependencies <group><package>_python_init})
    ##### **** statement *hlib.MakeFragmentStmt (&{python_init})
    #### macro &{{python_bin_module_dir [{default [${tag}/python/lib-dynload]} {target-winxp [${tag}\python\lib-dynload]} {use-shared-dir&target-winxp [${tag}\python]} {use-shared-dir [${tag}/python]}]}}
    macro("python_bin_module_dir", (
      {"default": "${tag}/python/lib-dynload"},
      {"target-winxp": "${tag}\\python\\lib-dynload"},
      {("use-shared-dir", "target-winxp"): "${tag}\\python"},
      {"use-shared-dir": "${tag}/python"},
    ))
    ##### **** statement *hlib.PatternStmt (&{pyd_module macro <package>_install_pyd_module  <project>_without_installarea&target-unix $(<package>_root)/$(tag) <project>_without_installarea&target-winxp $(<package>_root)\$(tag) ; path_remove PYTHONPATH $(<package>_install_pyd_module) ; path_prepend PYTHONPATH $(<package>_install_pyd_module) ; macro <module>_shlibflags $(libraryshr_linkopts) $(cmt_installarea_linkopts) $(<module>_use_linkopts) macro_append <module>_dependencies <name> <deps> ; private ; macro shlibsuffix $(shlibsuffix) target_<module>&target-winxp pyd ; macro library_prefix $(library_prefix) target_<module>  ; macro <module>_install_dir $(CMTINSTALLAREA)/$(python_bin_module_dir) target-winxp $(CMTINSTALLAREA)\$(python_bin_module_dir) ; library <module> -no_static -import=Python -target_tag install_dir="$(<module>_install_dir) <files> ; end_private ; macro_append <module>_cppflags -ftemplate-depth-64 target-winxp  ;})
    ##### **** statement *hlib.PatternStmt (&{pyd_boost_module apply_pattern pyd_module module=<module> files=<files> deps=<deps> name=<name> ; macro_append <module>_use_linkopts $(Boost_linkopts_python)})
    #### macro_remove &{{cmt_actions_constituents [{default [CompilePython]}]}}
    ctx.hwaf_macro_remove("cmt_actions_constituents", (
      {"default": "CompilePython"},
    ))
    #### macro_remove &{{cmt_actions_constituentsclean [{default [CompilePythonclean]}]}}
    ctx.hwaf_macro_remove("cmt_actions_constituentsclean", (
      {"default": "CompilePythonclean"},
    ))
    ##### **** statement *hlib.PatternStmt (&{install_scripts macro <package>_install_scripts  <project>_without_installarea&target-unix $(<package>_root)/$(scripts_dir) <project>_without_installarea&target-winxp $(<package>_root)\$(scripts_dir) ; path_remove PATH ${<package>_install_scripts} ; path_prepend PATH ${<package>_install_scripts} ; document install_scripts install_scripts source=$(scripts_dir) offset=$(scripts_offset)})
    ##### **** statement *hlib.MakeFragmentStmt (&{install_scripts})
    #### macro &{{install_command [{default [python ${GaudiPolicy_root}/scripts/install.py -xCVS -x*~ -x*.stamp -x*.bak -x.* -x*.pyc -x*.pyo --log=./install.${tag}.history]}]}}
    macro("install_command", (
      {"default": ["python", "${GaudiPolicy_root}/scripts/install.py", "-xCVS", "-x*~", "-x*.stamp", "-x*.bak", "-x.*", "-x*.pyc", "-x*.pyo", "--log=./install.${tag}.history"]},
    ))
    #### macro &{{uninstall_command [{default [python ${GaudiPolicy_root}/scripts/install.py -u --log=./install.${tag}.history]}]}}
    macro("uninstall_command", (
      {"default": ["python", "${GaudiPolicy_root}/scripts/install.py", "-u", "--log=./install.${tag}.history"]},
    ))
    #### macro &{{remove_command [{default [${cmt_uninstallarea_command}]}]}}
    macro("remove_command", (
      {"default": "${cmt_uninstallarea_command}"},
    ))
    #### macro &{{library_install_command [{default [python ${GaudiPolicy_root}/scripts/install.py -xCVS -x*~ -x*.stamp -x*.bak -x.* -x*.pyc -x*.pyo -s --log=./install.${tag}.history]} {host-winxp [copy]}]}}
    macro("library_install_command", (
      {"default": ["python", "${GaudiPolicy_root}/scripts/install.py", "-xCVS", "-x*~", "-x*.stamp", "-x*.bak", "-x.*", "-x*.pyc", "-x*.pyo", "-s", "--log=./install.${tag}.history"]},
      {"host-winxp": "copy"},
    ))
    #### macro &{{cmt_installarea_command [{default [python ${GaudiPolicy_root}/scripts/install.py -xCVS -x*~ -x*.stamp -x*.bak -x.* -x*.pyc -x*.pyo -s --log=./install.${tag}.history]} {host-winxp [xcopy /S /E /Y]}]}}
    macro("cmt_installarea_command", (
      {"default": ["python", "${GaudiPolicy_root}/scripts/install.py", "-xCVS", "-x*~", "-x*.stamp", "-x*.bak", "-x.*", "-x*.pyc", "-x*.pyo", "-s", "--log=./install.${tag}.history"]},
      {"host-winxp": ["xcopy", "/S", "/E", "/Y"]},
    ))
    ##### **** statement *hlib.PatternStmt (&{declare_runtime macro_append <package>_runtime -s=../share <files> <extras> ; macro_append use_runtime $(<package>_runtime) ; document installer install_<name>runtime $(<package>_runtime) install_dir=$(CMTINSTALLAREA)/share})
    ##### **** statement *hlib.PatternStmt (&{declare_runtime_extras apply_pattern declare_runtime files="<files> extras="<extras> name="<name>})
    ##### **** statement *hlib.PatternStmt (&{install_runtime macro data $(use_runtime) ; document get_files <name>get_files install_dir=../run/ copymethod=<method>})
    ##### **** statement *hlib.MakeFragmentStmt (&{get_files})
    ##### **** statement *hlib.PatternStmt (&{get_files macro <name>jo <jo> ; macro <name>scripts <scripts> ; macro <name>data <data> ; macro <name>xmls <xmls> ; document get_files <name>get_files install_dir=../run/ copymethod=<method> name=<name>})
    ##### **** statement *hlib.PatternStmt (&{package_stamps apply_pattern library_stamps library="<package>})
    ##### **** statement *hlib.PatternStmt (&{package_linkopts apply_pattern library_Llinkopts library="<package>})
    ##### **** statement *hlib.PatternStmt (&{package_shlibflags apply_pattern library_Lshlibflags library="<package>})
    ##### **** statement *hlib.PatternStmt (&{package_libraries apply_pattern library_Softlinks library="<package> <package>Lib <package>Dict})
    ##### **** statement *hlib.PatternStmt (&{packageShr apply_pattern libraryShr library="<package>})
    ##### **** statement *hlib.PatternStmt (&{package_Cshlibflags apply_pattern library_Cshlibflags library="<package>})
    ##### **** statement *hlib.PatternStmt (&{package_Lstamps apply_pattern library_stamps library="<package>Lib})
    ##### **** statement *hlib.PatternStmt (&{package_Llinkopts apply_pattern library_Llinkopts library="<package>Lib})
    ##### **** statement *hlib.PatternStmt (&{package_Lshlibflags apply_pattern library_shlibflags library="<package>Lib})
    ##### **** statement *hlib.PatternStmt (&{package_Slinkopts macro <package>_linkopts $(<package>Dir)/lib<package>.a -u <package>_loadRef target-winxp $(<package>Dir)/<package>.lib /include:_<package>_loadRef })
    ##### **** statement *hlib.PatternStmt (&{package_Flinkopts macro <package>_linkopts $(<package>Dir)/lib<package>.a target-winxp $(<package>Dir)/<package>.lib })
    #### macro &{{dq [{default []}]}}
    macro("dq", (
      {"default": ""},
    ))
    ##### **** statement *hlib.PatternStmt (&{package_fflags macro <package>_fflags  target-winxp /fpp:$(dq)/I$(<PACKAGE>ROOT)$(dq) /fpp:$(dq)/I$(<PACKAGE>ROOT)/src$(dq)})
    ##### **** statement *hlib.PatternStmt (&{ld_library_path path_remove LD_LIBRARY_PATH /<package>/ target-winxp  ; path_append LD_LIBRARY_PATH <project>_without_installarea&target-unix ${<package>_root}/${<package>_tag} ; path_remove PATH  target-winxp \<package>" ; path_prepend PATH  <project>_without_installarea&target-winxp ${<package>_root}/${<package>_tag} ; path_remove DYLD_LIBRARY_PATH  target-darwin /<package>/ ; path_append DYLD_LIBRARY_PATH  <project>_without_installarea&target-darwin ${<package>_root}/${<package>_tag}})
    ##### **** statement *hlib.PatternStmt (&{application_path path_remove LD_LIBRARY_PATH /<package>/ target-winxp  ; path_prepend LD_LIBRARY_PATH  <project>_without_installarea&target-unix ${<package>_root}/$(<package>_tag)})
    ##### **** statement *hlib.PatternStmt (&{generate_rootmap document genmap <library>RootMap -group=<group> LIBNAME=<library> ; macro_append <library>RootMap_dependencies <library> ; private ; apply_pattern application_path ; macro merge_rootmap_tag --do-merge do_merge_rootmap --do-merge no_merge_rootmap --un-merge ; document merge_rootmap <library>MergeMap -group=<group> library=<library> merge_rootmap_switch=$(merge_rootmap_tag) $(bin)$(library_prefix)<library>.$(shlibsuffix) ; macro_append <library>MergeMap_dependencies <library>RootMap ; end_private})

    #### macro &{{BINDIR [{default [${tag}]}]}}
    #macro("BINDIR", (
    #  {"default": "${CMTCFG}"},
    #))

    #### tag &{host-darwin [host-unix]}
    ctx.hwaf_declare_tag(
        "host-darwin",
        content=["host-unix"]
    )
    #### tag &{sl51 [host-slc5]}
    ctx.hwaf_declare_tag(
        "sl51",
        content=["host-slc5"]
    )
    #### tag &{sl52 [host-slc5]}
    ctx.hwaf_declare_tag(
        "sl52",
        content=["host-slc5"]
    )
    #### tag &{sl53 [host-slc5]}
    ctx.hwaf_declare_tag(
        "sl53",
        content=["host-slc5"]
    )
    #### tag &{sl54 [host-slc5]}
    ctx.hwaf_declare_tag(
        "sl54",
        content=["host-slc5"]
    )
    #### tag &{sl55 [host-slc5]}
    ctx.hwaf_declare_tag(
        "sl55",
        content=["host-slc5"]
    )
    #### tag &{sl56 [host-slc5]}
    ctx.hwaf_declare_tag(
        "sl56",
        content=["host-slc5"]
    )
    #### tag &{sl57 [host-slc5]}
    ctx.hwaf_declare_tag(
        "sl57",
        content=["host-slc5"]
    )
    #### tag &{ATLAS [use-shared-dir]}
    ctx.hwaf_declare_tag(
        "ATLAS",
        content=["use-shared-dir"]
    )
    ##### **** statement *hlib.MakeFragmentStmt (&{dsp_application_header})
    ##### **** statement *hlib.MakeFragmentStmt (&{dsp_library_header})
    ##### **** statement *hlib.MakeFragmentStmt (&{dsp_all})
    ##### **** statement *hlib.MakeFragmentStmt (&{rule_checker})
    ##### **** statement *hlib.MakeFragmentStmt (&{rule_checker_trailer})
    ##### **** statement *hlib.MakeFragmentStmt (&{vcproj_library_header})
    ##### **** statement *hlib.MakeFragmentStmt (&{vcproj_application_header})
    ##### **** statement *hlib.MakeFragmentStmt (&{vcproj_all})
    ##### **** statement *hlib.MakeFragmentStmt (&{vcproj_trailer})
    ##### **** statement *hlib.MakeFragmentStmt (&{vcproj_contents})
    ##### **** statement *hlib.MakeFragmentStmt (&{vcproj_directory_header})
    ##### **** statement *hlib.MakeFragmentStmt (&{vcproj_directory_trailer})
    ##### **** statement *hlib.MakeFragmentStmt (&{sln_header})
    ##### **** statement *hlib.MakeFragmentStmt (&{sln_project})
    ##### **** statement *hlib.MakeFragmentStmt (&{sln_trailer})
    ##### **** statement *hlib.MakeFragmentStmt (&{sln_project_config})
    ##### **** statement *hlib.MakeFragmentStmt (&{sln_dependency_project})
    ##### **** statement *hlib.MakeFragmentStmt (&{sln_dependency_header})
    ##### **** statement *hlib.MakeFragmentStmt (&{sln_dependency_trailer})
    #### macro &{{cpp_fragment [{default [cpp]} {target-winxp []} {CMTv1&CMTr25 []}]}}
    macro("cpp_fragment", (
      {"default": "cpp"},
      {"target-winxp": ""},
      {("CMTv1", "CMTr25"): ""},
    ))
    ##### **** statement *hlib.MakeFragmentStmt (&{$(cpp_fragment)})
    #### macro &{{cpp_library_fragment [{default [cpp_library]} {target-winxp []} {CMTv1&CMTr25 []}]}}
    macro("cpp_library_fragment", (
      {"default": "cpp_library"},
      {"target-winxp": ""},
      {("CMTv1", "CMTr25"): ""},
    ))
    ##### **** statement *hlib.MakeFragmentStmt (&{$(cpp_library_fragment)})
    #### tag &{CMTv1&CMTr25 [c_native_dependencies cpp_native_dependencies]}
    ctx.hwaf_declare_tag(
        "CMTv1&CMTr25",
        content=["c_native_dependencies", "cpp_native_dependencies"]
    )
    #### macro &{{cmt_compiler_version [{default [${cmt_compiler_version}]} {target-gcc43 [gcc43]}]}}
    macro("cmt_compiler_version", (
      {"default": "${cmt_compiler_version}"},
      {"target-gcc43": "gcc43"},
    ))
    #### macro &{{cppcomp [{default [${cpp} -c ${cppdebugflags} ${cppflags} ${pp_cppflags} ${includes}]} {VisualC [cl.exe ${cppdebugflags} ${cppflags} ${pp_cppflags}]}]}}
    macro("CXXcomp", (
      {"default": ["${CXX}", "-c", "${CXXdebugflags}", "${CXXflags}", "${pp_CXXflags}", "${includes}"]},
      {"VisualC": ["cl.exe", "${CXXdebugflags}", "${CXXflags}", "${pp_CXXflags}"]},
    ))
    #### macro &{{dep_fragment [{default []} {CMTp20070208 [dependencies]}]}}
    macro("dep_fragment", (
      {"default": ""},
      {"CMTp20070208": "dependencies"},
    ))
    ##### **** statement *hlib.MakeFragmentStmt (&{$(dep_fragment)})
    #### macro &{{app_fragment [{default []} {target-winxp&target-vc9 [application]}]}}
    macro("app_fragment", (
      {"default": ""},
      {("target-winxp", "target-vc9"): "application"},
    ))
    ##### **** statement *hlib.MakeFragmentStmt (&{$(app_fragment)})
    #### macro &{{vsCONFIG [{default [Release]} {target-dbg [Debug]}]}}
    macro("vsCONFIG", (
      {"default": "Release"},
      {"target-dbg": "Debug"},
    ))
    #### macro &{{vsDebug [{default [2]} {target-dbg [1]}]}}
    macro("vsDebug", (
      {"default": "2"},
      {"target-dbg": "1"},
    ))
    #### macro &{{vsOptimize [{default [2]} {target-dbg [0]}]}}
    macro("vsOptimize", (
      {"default": "2"},
      {"target-dbg": "0"},
    ))
    #### macro &{{vsVersion [{default []} {vc++7.1 [7.10]} {vc++7.0 [7.00]} {vc++9 [7.10]}]}}
    macro("vsVersion", (
      {"default": ""},
      {"vc++7.1": "7.10"},
      {"vc++7.0": "7.00"},
      {"vc++9": "7.10"},
    ))
    #### macro &{{package_GUID [{default [{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}]}]}}
    macro("package_GUID", (
      {"default": "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}"},
    ))
    #### macro &{{GUID_all [{default [{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC955}]}]}}
    macro("GUID_all", (
      {"default": "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC955}"},
    ))
    #### macro &{{make_shlib [{default [echo]} {HP-UX [${CMTROOT}/mgr/cmt_make_shlib_common.sh extract]} {OSF1 [${CMTROOT}/mgr/cmt_make_shlib_common.sh noextract]} {host-linux [${CMTROOT}/mgr/cmt_make_shlib_common.sh extract]} {CYGWIN [${CMTROOT}/mgr/cmt_make_shlib_common.sh extract]} {SunOS [${CMTROOT}/mgr/cmt_make_shlib_common.sh extract]} {host-darwin [${CMTROOT}/mgr/cmt_make_shlib_common.sh extract]} {host-visualc [${GAUDIPOLICYROOT}\cmt\cmt_make_shlib.bat]}]}}
    macro("make_shlib", (
      {"default": "echo"},
      {"HP-UX": ["${CMTROOT}/mgr/cmt_make_shlib_common.sh", "extract"]},
      {"OSF1": ["${CMTROOT}/mgr/cmt_make_shlib_common.sh", "noextract"]},
      {"host-linux": ["${CMTROOT}/mgr/cmt_make_shlib_common.sh", "extract"]},
      {"CYGWIN": ["${CMTROOT}/mgr/cmt_make_shlib_common.sh", "extract"]},
      {"SunOS": ["${CMTROOT}/mgr/cmt_make_shlib_common.sh", "extract"]},
      {"host-darwin": ["${CMTROOT}/mgr/cmt_make_shlib_common.sh", "extract"]},
      {"host-visualc": "${GAUDIPOLICYROOT}\\cmt\\cmt_make_shlib.bat"},
    ))
    #### macro &{{merge_rootmap_cmd [{default [python ${GaudiPolicy_root}/scripts/merge_files.py]} {target-winxp [${GaudiPolicy_root}/scripts/merge_files.py]}]}}
    macro("merge_rootmap_cmd", (
      {"default": ["python", "${GaudiPolicy_root}/scripts/merge_files.py"]},
      {"target-winxp": "${GaudiPolicy_root}/scripts/merge_files.py"},
    ))
    ##### **** statement *hlib.MakeFragmentStmt (&{merge_rootmap})
    #### macro &{{libdirname [{default [lib]} {# [defaults]} {FC [fragments]} {of [libs]} {and [bins]}]}}
    macro("libdirname", (
      {"default": "lib"},
      {"#": "defaults"},
      {"for": "fragments"},
      {"of": "libs"},
      {"and": "bins"},
    ))
    #### macro &{{bindirname [{default [bin]} {# [defaults]} {for [fragments]} {of [libs]} {and [bins]}]}}
    macro("bindirname", (
      {"default": "bin"},
      {"#": "defaults"},
      {"for": "fragments"},
      {"of": "libs"},
      {"and": "bins"},
    ))
    #### macro &{{lib_fragment [{default []} {CMTp20070208 [library -header=library_header -dependencies]} {CMTp20090520&target-winxp [library -header=library_header -dependencies]}]}}
    macro("lib_fragment", (
      {"default": ""},
      {"CMTp20070208": ["library", "-header=library_header", "-dependencies"]},
      {("CMTp20090520", "target-winxp"): ["library", "-header=library_header", "-dependencies"]},
    ))
    ##### **** statement *hlib.MakeFragmentStmt (&{$(lib_fragment)})
    #### macro &{{lib_no_static_fragment [{default []} {CMTp20070208&target-winxp [library_no_static]} {CMTp20090520&target-winxp [library_no_static]} {CMTr20&target-unix []} {target-unix [library_no_static]}]}}
    macro("lib_no_static_fragment", (
      {"default": ""},
      {("CMTp20070208", "target-winxp"): "library_no_static"},
      {("CMTp20090520", "target-winxp"): "library_no_static"},
      {("CMTr20", "target-unix"): ""},
      {"target-unix": "library_no_static"},
    ))
    ##### **** statement *hlib.MakeFragmentStmt (&{$(lib_no_static_fragment)})
    #### macro_prepend &{{CXX_NAME [{default []} {lcg-compiler&target-gcc&CERNDISTCC []} {lcg-compiler&target-gccmax []} {lcg-compiler&target-g11max []} {lcg-compiler&target-gcc [lcg-]} {lcg-compiler&target-clang [lcg-]}]}}
    ctx.hwaf_macro_prepend("CXX_NAME", (
      {"default": ""},
      {("lcg-compiler", "target-gcc", "CERNDISTCC"): ""},
      {("lcg-compiler", "target-gccmax"): ""},
      {("lcg-compiler", "target-g11max"): ""},
      {("lcg-compiler", "target-gcc"): "lcg-"},
      {("lcg-compiler", "target-clang"): "lcg-"},
    ))
    #### macro_append &{{CXX_NAME [{default []} {lcg-compiler&target-gcc&CERNDISTCC []} {lcg-compiler&target-gccmax []} {lcg-compiler&target-g11max []} {lcg-compiler&target-gcc [-${gcc_config_version}]} {lcg-compiler&target-clang [-${clang_config_version}]}]}}
    ctx.hwaf_macro_append("CXX_NAME", (
      {"default": ""},
      {("lcg-compiler", "target-gcc", "CERNDISTCC"): ""},
      {("lcg-compiler", "target-gccmax"): ""},
      {("lcg-compiler", "target-g11max"): ""},
      {("lcg-compiler", "target-gcc"): "-${gcc_config_version}"},
      {("lcg-compiler", "target-clang"): "-${clang_config_version}"},
    ))
    #### macro &{{CXX_NAME [{default [${CXX_NAME}]} {target-icc [icpc]}]}}
    macro("CXX_NAME", (
      {"default": "${CXX_NAME}"},
      {"target-icc": "icpc"},
    ))
    #### macro_prepend &{{CC_NAME [{default []} {lcg-compiler&target-gcc&CERNDISTCC []} {lcg-compiler&target-gccmax []} {lcg-compiler&target-g11max []} {lcg-compiler&target-gcc [lcg-]} {lcg-compiler&target-clang [lcg-]}]}}
    ctx.hwaf_macro_prepend("CC_NAME", (
      {"default": ""},
      {("lcg-compiler", "target-gcc", "CERNDISTCC"): ""},
      {("lcg-compiler", "target-gccmax"): ""},
      {("lcg-compiler", "target-g11max"): ""},
      {("lcg-compiler", "target-gcc"): "lcg-"},
      {("lcg-compiler", "target-clang"): "lcg-"},
    ))
    #### macro_append &{{CC_NAME [{default []} {lcg-compiler&target-gcc&CERNDISTCC []} {lcg-compiler&target-gccmax []} {lcg-compiler&target-g11max []} {lcg-compiler&target-gcc [-${gcc_config_version}]} {lcg-compiler&target-clang [-${clang_config_version}]}]}}
    ctx.hwaf_macro_append("CC_NAME", (
      {"default": ""},
      {("lcg-compiler", "target-gcc", "CERNDISTCC"): ""},
      {("lcg-compiler", "target-gccmax"): ""},
      {("lcg-compiler", "target-g11max"): ""},
      {("lcg-compiler", "target-gcc"): "-${gcc_config_version}"},
      {("lcg-compiler", "target-clang"): "-${clang_config_version}"},
    ))
    #### macro_prepend &{{FC_NAME [{default []} {lcg-compiler&target-gcc&CERNDISTCC []} {lcg-compiler&target-gccmax []} {lcg-compiler&target-g11max []} {lcg-compiler&target-gcc [lcg-]} {lcg-compiler&target-clang [lcg-]}]}}
    ctx.hwaf_macro_prepend("FC_NAME", (
      {"default": ""},
      {("lcg-compiler", "target-gcc", "CERNDISTCC"): ""},
      {("lcg-compiler", "target-gccmax"): ""},
      {("lcg-compiler", "target-g11max"): ""},
      {("lcg-compiler", "target-gcc"): "lcg-"},
      {("lcg-compiler", "target-clang"): "lcg-"},
    ))
    #### macro_append &{{FC_NAME [{default []} {lcg-compiler&target-gcc&CERNDISTCC []} {lcg-compiler&target-gccmax []} {lcg-compiler&target-g11max []} {lcg-compiler&target-gcc [-${gcc_config_version}]} {lcg-compiler&target-clang [-${gcc_config_version}]}]}}
    ctx.hwaf_macro_append("FC_NAME", (
      {"default": ""},
      {("lcg-compiler", "target-gcc", "CERNDISTCC"): ""},
      {("lcg-compiler", "target-gccmax"): ""},
      {("lcg-compiler", "target-g11max"): ""},
      {("lcg-compiler", "target-gcc"): "-${gcc_config_version}"},
      {("lcg-compiler", "target-clang"): "-${gcc_config_version}"},
    ))
    #### macro &{{cppdebugflags [{default []} {target-dbg [${cppdebugflags_s} target-opt ${cppoptimized_s}]}]}}
    macro("CXXdebugflags", (
      {"default": ""},
      {"target-dbg": ["${CXXdebugflags_s}", "target-opt", "${CXXoptimized_s}"]},
    ))
    #### macro &{{cppdebugflags_s [{default [-g]} {target-gcc48 [-Og -g]} {target-winxp [/Od /Z7]}]}}
    macro("CXXdebugflags_s", (
      {"default": "-g"},
      {"target-gcc48": ["-Og", "-g"]},
      {"target-winxp": ["/Od", "/Z7"]},
    ))
    #### macro &{{cppoptimized_s [{default [-O2 -DNDEBUG]} {use-O3&target-gcc [-O3 -DNDEBUG]} {target-winxp []}]}}
    macro("CXXoptimized_s", (
      {"default": ["-O2", "-DNDEBUG"]},
      {("use-O3", "target-gcc"): ["-O3", "-DNDEBUG"]},
      {"target-winxp": ""},
    ))
    #### macro &{{cppprofiled_s [{default [-pg]} {target-winxp []}]}}
    macro("CXXprofiled_s", (
      {"default": "-pg"},
      {"target-winxp": ""},
    ))
    #### macro_append &{{cppdebugflags [{default []} {target-pro [${cppprofiled_s}]}]}}
    ctx.hwaf_macro_append("CXXdebugflags", (
      {"default": ""},
      {"target-pro": "${CXXprofiled_s}"},
    ))
    #### macro &{{cdebugflags [{default []} {target-dbg [${cdebugflags_s}]} {target-opt [${coptimized_s}]}]}}
    macro("cdebugflags", (
      {"default": ""},
      {"target-dbg": "${cdebugflags_s}"},
      {"target-opt": "${coptimized_s}"},
    ))
    #### macro &{{cdebugflags_s [{default [-g]} {target-gcc48 [-Og -g]} {target-winxp [/Od /Z7]}]}}
    macro("cdebugflags_s", (
      {"default": "-g"},
      {"target-gcc48": ["-Og", "-g"]},
      {"target-winxp": ["/Od", "/Z7"]},
    ))
    #### macro &{{coptimized_s [{default [-O2 -DNDEBUG]} {use-O3&target-gcc [-O3 -DNDEBUG]} {target-winxp []}]}}
    macro("coptimized_s", (
      {"default": ["-O2", "-DNDEBUG"]},
      {("use-O3", "target-gcc"): ["-O3", "-DNDEBUG"]},
      {"target-winxp": ""},
    ))
    #### macro &{{fdebugflags [{default []} {target-dbg [${fdebugflags_s}]}]}}
    macro("fdebugflags", (
      {"default": ""},
      {"target-dbg": "${fdebugflags_s}"},
    ))
    #### macro &{{fdebugflags_s [{default [-g target-gcc48 -Og]} {-g [target-winxp]}]}}
    macro("fdebugflags_s", (
      {"default": ["-g", "target-gcc48", "-Og"]},
      {"-g": "target-winxp"},
    ))
    #### macro &{{foptimized_s [{default [-O2 -DNDEBUG]} {use-O3&target-gcc [-O3 -DNDEBUG]} {target-winxp []}]}}
    macro("foptimized_s", (
      {"default": ["-O2", "-DNDEBUG"]},
      {("use-O3", "target-gcc"): ["-O3", "-DNDEBUG"]},
      {"target-winxp": ""},
    ))
    #### macro_append &{{fdebugflags [{default []} {target-opt [${foptimized_s}]}]}}
    ctx.hwaf_macro_append("fdebugflags", (
      {"default": ""},
      {"target-opt": "${foptimized_s}"},
    ))
    #### macro &{{fprofiled_s [{default [-pg]} {target-winxp []}]}}
    macro("fprofiled_s", (
      {"default": "-pg"},
      {"target-winxp": ""},
    ))
    #### macro_append &{{fdebugflags [{default []} {target-pro [${fprofiled_s}]}]}}
    ctx.hwaf_macro_append("fdebugflags", (
      {"default": ""},
      {"target-pro": "${fprofiled_s}"},
    ))
    #### macro &{{linkdebugflags [{default []} {target-dbg [${linkdebugflags_s}]}]}}
    macro("linkdebugflags", (
      {"default": ""},
      {"target-dbg": "${linkdebugflags_s}"},
    ))
    #### macro &{{linkdebugflags_s [{default []} {target-winxp [/debug /verbose:lib]}]}}
    macro("linkdebugflags_s", (
      {"default": ""},
      {"target-winxp": ["/debug", "/verbose:lib"]},
    ))
    #### macro_append &{{LCG_Platforms_linkopts [{default []} {target-cov [--coverage]}]}}
    ctx.hwaf_macro_append("LCG_Platforms_linkopts", (
      {"default": ""},
      {"target-cov": "--coverage"},
    ))
    #### macro_append &{{LCG_Platforms_cppflags [{default []} {target-cov [--coverage]}]}}
    ctx.hwaf_macro_append("LCG_Platforms_cppflags", (
      {"default": ""},
      {"target-cov": "--coverage"},
    ))
    #### macro_append &{{LCG_Platforms_cflags [{default []} {target-cov [--coverage]}]}}
    ctx.hwaf_macro_append("LCG_Platforms_cflags", (
      {"default": ""},
      {"target-cov": "--coverage"},
    ))
    #### macro_append &{{LCG_Platforms_fflags [{default []} {target-cov [--coverage]}]}}
    ctx.hwaf_macro_append("LCG_Platforms_fflags", (
      {"default": ""},
      {"target-cov": "--coverage"},
    ))
    #### macro &{{cppflags [{default [-fmessage-length=0 -Df2cFortran -fPIC -D_GNU_SOURCE -Dlinux -Dunix -pipe -ansi -Wall -Wextra -Werror=return-type -pthread]} {target-mac105 [-Df2cFortran -fPIC -D_GNU_SOURCE -Dunix -pipe -ansi -Wall -Wextra -Werror=return-type -Wno-long-double]} {target-mac106 [-Df2cFortran -fPIC -D_GNU_SOURCE -Dunix -pipe -ansi -Wall -Wextra -Werror=return-type]} {target-vc9 [${includes} /D WIN32 /D _MBCS /D _WINDOWS /FD /c /nologo /W3 /EHsc /MD /GR /Zm500]} {target-vc7 [${includes} /D WIN32 /D _MBCS /D _WINDOWS /FD /c /nologo /W3 /GX /MD /GR /Zm500]}]}}
    macro("CXXflags", (
      {"default": ["-fmessage-length=0", "-Df2cFortran", "-fPIC", "-D_GNU_SOURCE", "-Dlinux", "-Dunix", "-pipe", "-ansi", "-Wall", "-Wextra", "-Werror=return-type", "-pthread"]},
      {"target-mac105": ["-Df2cFortran", "-fPIC", "-D_GNU_SOURCE", "-Dunix", "-pipe", "-ansi", "-Wall", "-Wextra", "-Werror=return-type", "-Wno-long-double"]},
      {"target-mac106": ["-Df2cFortran", "-fPIC", "-D_GNU_SOURCE", "-Dunix", "-pipe", "-ansi", "-Wall", "-Wextra", "-Werror=return-type"]},
      {"target-vc9": ["${includes}", "/D", "WIN32", "/D", "_MBCS", "/D", "_WINDOWS", "/FD", "/c", "/nologo", "/W3", "/EHsc", "/MD", "/GR", "/Zm500"]},
      {"target-vc7": ["${includes}", "/D", "WIN32", "/D", "_MBCS", "/D", "_WINDOWS", "/FD", "/c", "/nologo", "/W3", "/GX", "/MD", "/GR", "/Zm500"]},
    ))
    #### macro_append &{{cppflags [{default []} {target-gcc43 []} {GAUDI_PEDANTIC&target-gcc [-pedantic -Wno-long-long]}]}}
    ctx.hwaf_macro_append("CXXflags", (
      {"default": ""},
      {"target-gcc43": ""},
      {("GAUDI_PEDANTIC", "target-gcc"): ["-pedantic", "-Wno-long-long"]},
    ))
    #### macro_remove &{{cppflags [{default []} {target-icc [-pedantic]}]}}
    ctx.hwaf_macro_remove("CXXflags", (
      {"default": ""},
      {"target-icc": "-pedantic"},
    ))
    #### macro_remove &{{cppflags [{default []} {target-icc [-Wextra]}]}}
    ctx.hwaf_macro_remove("CXXflags", (
      {"default": ""},
      {"target-icc": "-Wextra"},
    ))
    #### macro_remove &{{cppflags [{default []} {target-icc [-Wno-long-long]}]}}
    ctx.hwaf_macro_remove("CXXflags", (
      {"default": ""},
      {"target-icc": "-Wno-long-long"},
    ))
    #### macro_remove &{{cppflags [{default []} {target-icc [-Werror=return-type]}]}}
    ctx.hwaf_macro_remove("CXXflags", (
      {"default": ""},
      {"target-icc": "-Werror=return-type"},
    ))
    #### macro_append &{{cppflags [{default []} {target-icc [-wd383]}]}}
    ctx.hwaf_macro_append("CXXflags", (
      {"default": ""},
      {"target-icc": "-wd383"},
    ))
    #### macro_append &{{cppflags [{default []} {target-icc [-wd981]}]}}
    ctx.hwaf_macro_append("CXXflags", (
      {"default": ""},
      {"target-icc": "-wd981"},
    ))
    #### macro_append &{{cppflags [{default []} {target-icc [-wd1418]}]}}
    ctx.hwaf_macro_append("CXXflags", (
      {"default": ""},
      {"target-icc": "-wd1418"},
    ))
    #### macro_append &{{cppflags [{default []} {target-icc [-wd1419]}]}}
    ctx.hwaf_macro_append("CXXflags", (
      {"default": ""},
      {"target-icc": "-wd1419"},
    ))
    #### macro_append &{{cppflags [{default []} {target-icc [-we1011 -we117]}]}}
    ctx.hwaf_macro_append("CXXflags", (
      {"default": ""},
      {"target-icc": ["-we1011", "-we117"]},
    ))
    #### macro_append &{{cppflags [{default []} {no-hide-warnings []} {target-icc [-wd82]}]}}
    ctx.hwaf_macro_append("CXXflags", (
      {"default": ""},
      {"no-hide-warnings": ""},
      {"target-icc": "-wd82"},
    ))
    #### macro_append &{{cppflags [{default []} {no-hide-warnings []} {target-icc [-wd522]}]}}
    ctx.hwaf_macro_append("CXXflags", (
      {"default": ""},
      {"no-hide-warnings": ""},
      {"target-icc": "-wd522"},
    ))
    #### macro_append &{{cppflags [{default []} {no-hide-warnings []} {target-icc [-wd444]}]}}
    ctx.hwaf_macro_append("CXXflags", (
      {"default": ""},
      {"no-hide-warnings": ""},
      {"target-icc": "-wd444"},
    ))
    #### macro_append &{{cppflags [{default []} {no-hide-warnings []} {target-icc [-wd304]}]}}
    ctx.hwaf_macro_append("CXXflags", (
      {"default": ""},
      {"no-hide-warnings": ""},
      {"target-icc": "-wd304"},
    ))
    #### macro_append &{{cppflags [{default []} {no-hide-warnings []} {target-icc [-Wno-deprecated]}]}}
    ctx.hwaf_macro_append("CXXflags", (
      {"default": ""},
      {"no-hide-warnings": ""},
      {"target-icc": "-Wno-deprecated"},
    ))
    #### macro_append &{{GaudiKernel_linkopts [{default []} {target-icc [-limf -lm]}]}}
    ctx.hwaf_macro_append("GaudiKernel_linkopts", (
      {"default": ""},
      {"target-icc": ["-limf", "-lm"]},
    ))
    #### macro_append &{{cppflags [{default []} {GAUDI_V21&target-gcc4 [-fvisibility=hidden -fvisibility-inlines-hidden]} {G21_HIDE_SYMBOLS&target-gcc4 [-fvisibility=hidden -fvisibility-inlines-hidden]}]}}
    ctx.hwaf_macro_append("CXXflags", (
      {"default": ""},
      {("GAUDI_V21", "target-gcc4"): ["-fvisibility=hidden", "-fvisibility-inlines-hidden"]},
      {("G21_HIDE_SYMBOLS", "target-gcc4"): ["-fvisibility=hidden", "-fvisibility-inlines-hidden"]},
    ))
    #### macro_append &{{pp_cppflags [{default [-DGAUDI_V20_COMPAT]} {GAUDI_V21 []} {GAUDI_V22 [-DGAUDI_V22_API]}]}}
    ctx.hwaf_macro_append("DEFINES", (
      {"default": "GAUDI_V20_COMPAT"},
      {"GAUDI_V21": ""},
      {"GAUDI_V22": "GAUDI_V22_API"},
    ))
    #### macro_append &{{pp_cppflags [{default []} {G21_NEW_INTERFACES [-DG21_NEW_INTERFACES]}]}}
    ctx.hwaf_macro_append("DEFINES", (
      {"default": ""},
      {"G21_NEW_INTERFACES": "G21_NEW_INTERFACES"},
    ))
    #### macro_append &{{pp_cppflags [{default []} {G21_HIDE_SYMBOLS [-DG21_HIDE_SYMBOLS]}]}}
    ctx.hwaf_macro_append("DEFINES", (
      {"default": ""},
      {"G21_HIDE_SYMBOLS": "G21_HIDE_SYMBOLS"},
    ))
    #### macro_append &{{pp_cppflags [{default []} {G21_NO_ENDREQ [-DG21_NO_ENDREQ]}]}}
    ctx.hwaf_macro_append("DEFINES", (
      {"default": ""},
      {"G21_NO_ENDREQ": "G21_NO_ENDREQ"},
    ))
    #### macro_append &{{pp_cppflags [{default []} {G21_NO_DEPRECATED [-DG21_NO_DEPRECATED]}]}}
    ctx.hwaf_macro_append("DEFINES", (
      {"default": ""},
      {"G21_NO_DEPRECATED": "G21_NO_DEPRECATED"},
    ))
    #### macro_append &{{pp_cppflags [{default []} {G22_NEW_SVCLOCATOR [-DG22_NEW_SVCLOCATOR]}]}}
    ctx.hwaf_macro_append("DEFINES", (
      {"default": ""},
      {"G22_NEW_SVCLOCATOR": "G22_NEW_SVCLOCATOR"},
    ))
    #### macro_append &{{pp_cppflags [{default []} {G22_NO_DEPRECATED [-DG22_NO_DEPRECATED]}]}}
    ctx.hwaf_macro_append("DEFINES", (
      {"default": ""},
      {"G22_NO_DEPRECATED": "G22_NO_DEPRECATED"},
    ))
    #### macro_append &{{cppflags [{default []} {host-x86_64&target-i386 [-m32]} {host-x86_64&target-i686 [-m32]} {host-darwin&target-i386 [-m32]} {host-i686&target-x86_64 [-m64]} {host-i386&target-x86_64 [-m64]}]}}
    ctx.hwaf_macro_append("CXXFLAGS", (
      {"default": ""},
      {("host-x86_64", "target-i386"): "-m32"},
      {("host-x86_64", "target-i686"): "-m32"},
      {("host-darwin", "target-i386"): "-m32"},
      {("host-i686", "target-x86_64"): "-m64"},
      {("host-i386", "target-x86_64"): "-m64"},
    ))
    #### macro_append &{{cppflags [{default []} {target-c11 [-std=c++11]}]}}
    ctx.hwaf_macro_append("CXXFLAGS", (
      {"default": ""},
      {"target-c11": "-std=c++11"},
    ))
    #### macro_append &{{cflags [{default []} {target-c11 [-std=c11]}]}}
    ctx.hwaf_macro_append("CFLAGS", (
      {"default": ""},
      {"target-c11": "-std=c11"},
    ))
    #### macro_append &{{pp_cppflags [{default [-DBOOST_FILESYSTEM_VERSION=3]}]}}
    ctx.hwaf_macro_append("DEFINES", (
      {"default": "BOOST_FILESYSTEM_VERSION=3"},
    ))
    #### macro_append &{{cppflags [{default []} {no-hide-warnings []} {target-gcc43 [-Wno-deprecated]} {target-gcc46 [-Wno-deprecated]} {target-gcc47 [-Wno-deprecated]} {target-gcc48 [-Wno-deprecated]} {target-gccmax [-Wno-deprecated]} {target-clang [-Wno-deprecated]}]}}
    ctx.hwaf_macro_append("CXXFLAGS", (
      {"default": ""},
      {"no-hide-warnings": ""},
      {"target-gcc43": "-Wno-deprecated"},
      {"target-gcc46": "-Wno-deprecated"},
      {"target-gcc47": "-Wno-deprecated"},
      {"target-gcc48": "-Wno-deprecated"},
      {"target-gccmax": "-Wno-deprecated"},
      {"target-clang": "-Wno-deprecated"},
    ))
    #### macro_append &{{cppflags [{default []} {no-hide-warnings []} {target-gcc43 [-Wno-empty-body]}]}}
    ctx.hwaf_macro_append("CXXFLAGS", (
      {"default": ""},
      {"no-hide-warnings": ""},
      {"target-gcc43": "-Wno-empty-body"},
    ))
    #### macro_append &{{cppflags [{default []} {no-hide-warnings []} {target-clang [-Wno-overloaded-virtual -Wno-char-subscripts]}]}}
    ctx.hwaf_macro_append("CXXFLAGS", (
      {"default": ""},
      {"no-hide-warnings": ""},
      {"target-clang": ["-Wno-overloaded-virtual", "-Wno-char-subscripts"]},
    ))
    #### macro_append &{{gccxml_cppflags [{default []} {target-gccmax [-D__STRICT_ANSI__]} {target-gcc47 [-D__STRICT_ANSI__]}]}}
    ctx.hwaf_macro_append("gccxml_cppflags", (
      {"default": ""},
      {"target-gccmax": "-D__STRICT_ANSI__"},
      {"target-gcc47": "-D__STRICT_ANSI__"},
    ))
    #### macro &{{ppcmd [{default [-I]}]}}
    macro("ppcmd", "-I")

    #### macro &{{FC_NAME [{default [gfortran]} {target-slc4 [g77]} {target-darwin [gfortran -ff2c]} {target-winxp [f77.exe]}]}}
    macro("FC_NAME", (
      {"default": "gfortran"},
      {"target-slc4": "g77"},
      {"target-darwin": ["gfortran", "-ff2c"]},
      {"target-winxp": "f77.exe"},
    ))
    #### macro &{{for [{default [${FC_NAME}]}]}}
    macro("FC", (
      {"default": "${FC_NAME}"},
    ))
    #### macro_append &{{shlibflags [{default [host-x86_64&target-i686]} {-m32 [host-x86_64&target-i386]} {-m32 [host-darwin&target-i386]} {-m32 [host-i686&target-x86_64]} {-m64 [host-i386&target-x86_64]}]}}
    ctx.hwaf_macro_append("shlibflags", (
      {"default": "host-x86_64&target-i686"},
      {"-m32": "host-x86_64&target-i386"},
      {"-m32": "host-darwin&target-i386"},
      {"-m32": "host-i686&target-x86_64"},
      {"-m64": "host-i386&target-x86_64"},
    ))
    #### macro &{{fflags [{default [-fmessage-length=0 -O2 -fdollar-ok -ff90 -w -fPIC]} {target-gcc4 [-fmessage-length=0 -O2 -fdollar-ok -w -fPIC]} {target-darwin [-fno-automatic -fdollar-ok -w]} {target-winxp [/compile_only /nologo /warn:nofileopt /warn:nouncalled /fpp:"/m /fpp:"/I..]}]}}
    macro("FCFLAGS", (
      {"default": ["-fmessage-length=0", "-O2", "-fdollar-ok", "-ff90", "-w", "-fPIC"]},
      {"target-gcc4": ["-fmessage-length=0", "-O2", "-fdollar-ok", "-w", "-fPIC"]},
      {"target-darwin": ["-fno-automatic", "-fdollar-ok", "-w"]},
      {"target-winxp": ["/compile_only", "/nologo", "/warn:nofileopt", "/warn:nouncalled", "/fpp:\"/m", "/fpp:\"/I.."]},
    ))
    #### macro_append &{{fflags [{default []} {host-x86_64&target-i686 [-m32]} {host-x86_64&target-i386 [-m32]} {host-darwin&target-i386 [-m32]} {host-i686&target-x86_64 [-m64]} {host-i386&target-x86_64 [-m64]}]}}
    ctx.hwaf_macro_append("FCFLAGS", (
      {"default": ""},
      {("host-x86_64", "target-i686"): "-m32"},
      {("host-x86_64", "target-i386"): "-m32"},
      {("host-darwin", "target-i386"): "-m32"},
      {("host-i686", "target-x86_64"): "-m64"},
      {("host-i386", "target-x86_64"): "-m64"},
    ))
    #### macro &{{fcomp [{default [${for} -c ${fincludes} ${fdebugflags} ${fflags} ${pp_fflags}]} {target-winxp [${for} ${fdebugflags} ${fflags} ${pp_fflags}]}]}}
    macro("fcomp", (
      {"default": ["${FC}", "-c", "${fincludes}", "${fdebugflags}", "${fflags}", "${pp_fflags}"]},
      {"target-winxp": ["${FC}", "${fdebugflags}", "${fflags}", "${pp_fflags}"]},
    ))
    #### macro_append &{{cflags [{default [-fmessage-length=0 -fPIC]} {target-winxp []}]}}
    ctx.hwaf_macro_append("CFLAGS", (
      {"default": ["-fmessage-length=0", "-fPIC"]},
      {"target-winxp": ""},
    ))
    #### macro_append &{{cflags [{default []} {host-x86_64&target-i686 [-m32]} {host-x86_64&target-i386 [-m32]} {host-darwin&target-i386 [-m32]} {host-i686&target-x86_64 [-m64]} {host-i386&target-x86_64 [-m64]}]}}
    ctx.hwaf_macro_append("CFLAGS", (
      {"default": ""},
      {("host-x86_64", "target-i686"): "-m32"},
      {("host-x86_64", "target-i386"): "-m32"},
      {("host-darwin", "target-i386"): "-m32"},
      {("host-i686", "target-x86_64"): "-m64"},
      {("host-i386", "target-x86_64"): "-m64"},
    ))
    #### macro &{{makeLinkMap [{default [target-linux -Wl,-Map,Linux.map]} {target-winxp [/map]}]}}
    macro("makeLinkMap", (
      {"default": ["target-linux", "-Wl,-Map,Linux.map"]},
      {"target-winxp": "/map"},
    ))
    #### macro &{{cpplinkflags [{default []} {target-linux [-Wl,-Bdynamic ${linkdebugflags} -ldl -Wl,--as-needed]} {target-vc [/nologo /machine:ix86 ${linkdebugflags} ${makeLinkMap} /nodefaultlib kernel32.lib user32.lib ws2_32.lib advapi32.lib shell32.lib msvcrt.lib msvcprt.lib oldnames.lib]}]}}
    macro("cpplinkflags", (
      {"default": ""},
      {"target-linux": ["-Wl,-Bdynamic", "${linkdebugflags}", "-ldl", "-Wl,--as-needed"]},
      {"target-vc": ["/nologo", "/machine:ix86", "${linkdebugflags}", "${makeLinkMap}", "/nodefaultlib", "kernel32.lib", "user32.lib", "ws2_32.lib", "advapi32.lib", "shell32.lib", "msvcrt.lib", "msvcprt.lib", "oldnames.lib"]},
    ))
    #### macro_append &{{cpplinkflags [{default []} {target-linux [-Wl,--no-undefined]}]}}
    ctx.hwaf_macro_append("cpplinkflags", (
      {"default": ""},
      {"target-linux": ["-Wl,--no-undefined"]},
    ))
    #### macro_remove &{{shlibflags [{default []} {target-mac [-undefined suppress]}]}}
    ctx.hwaf_macro_remove("shlibflags", (
      {"default": ""},
      {"target-mac": ["-undefined", "suppress"]},
    ))
    #### macro_append &{{cpplinkflags [{default []} {host-x86_64&target-i686 [-m32]} {host-x86_64&target-i386 [-m32]} {host-darwin&target-i386 [-m32]} {host-i686&target-x86_64 [-m64]} {host-i386&target-x86_64 [-m64]}]}}
    ctx.hwaf_macro_append("cpplinkflags", (
      {"default": ""},
      {("host-x86_64", "target-i686"): "-m32"},
      {("host-x86_64", "target-i386"): "-m32"},
      {("host-darwin", "target-i386"): "-m32"},
      {("host-i686", "target-x86_64"): "-m64"},
      {("host-i386", "target-x86_64"): "-m64"},
    ))
    #### macro_append &{{cpplinkflags [{default []} {target-cov [${linkdebugflags}]}]}}
    ctx.hwaf_macro_append("cpplinkflags", (
      {"default": ""},
      {"target-cov": "${linkdebugflags}"},
    ))
    #### macro_append &{{cpplinkflags [{default []} {target-pro [${cppprofiled_s}]}]}}
    ctx.hwaf_macro_append("cpplinkflags", (
      {"default": ""},
      {"target-pro": "${cppprofiled_s}"},
    ))
    #### macro &{{cpplink [{default [${cpp} ${cpplinkflags}]} {target-winxp [link.exe ${cpplinkflags}]}]}}
    macro("cpplink", (
      {"default": ["${cpp}", "${cpplinkflags}"]},
      {"target-winxp": ["link.exe", "${cpplinkflags}"]},
    ))
    #### macro &{{componentshr_linkopts [{default []} {target-linux [-fPIC -ldl -Wl,--as-needed]} {target-winxp [/DLL]}]}}
    macro("componentshr_linkopts", (
      {"default": ""},
      {"target-linux": ["-fPIC", "-ldl", "-Wl,--as-needed"]},
      {"target-winxp": "/DLL"},
    ))
    #### macro_append &{{componentshr_linkopts [{default []} {target-linux [-Wl,--no-undefined]}]}}
    ctx.hwaf_macro_append("componentshr_linkopts", (
      {"default": ""},
      {"target-linux": ["-Wl,--no-undefined"]},
    ))
    #### macro_append &{{componentshr_linkopts [{default []} {target-linux [-pthread]}]}}
    ctx.hwaf_macro_append("componentshr_linkopts", (
      {"default": ""},
      {"target-linux": "-pthread"},
    ))
    #### macro_append &{{componentshr_linkopts [{default []} {target-linux&target-opt&strip-symbols [-Wl,-s]}]}}
    ctx.hwaf_macro_append("componentshr_linkopts", (
      {"default": ""},
      {("target-linux", "target-opt", "strip-symbols"): ["-Wl,-s"]},
    ))
    #### macro_append &{{componentshr_linkopts [{default []} {target-cov [${linkdebugflags}]}]}}
    ctx.hwaf_macro_append("componentshr_linkopts", (
      {"default": ""},
      {"target-cov": "${linkdebugflags}"},
    ))
    #### macro_append &{{componentshr_linkopts [{default []} {host-x86_64&target-i686 [-m32]} {host-x86_64&target-i386 [-m32]} {host-darwin&target-i386 [-m32]} {host-i686&target-x86_64 [-m64]} {host-i386&target-x86_64 [-m64]}]}}
    ctx.hwaf_macro_append("componentshr_linkopts", (
      {"default": ""},
      {("host-x86_64", "target-i686"): "-m32"},
      {("host-x86_64", "target-i386"): "-m32"},
      {("host-darwin", "target-i386"): "-m32"},
      {("host-i686", "target-x86_64"): "-m64"},
      {("host-i386", "target-x86_64"): "-m64"},
    ))
    #### macro &{{libraryshr_linkopts [{default []} {target-linux [-fPIC -ldl -Wl,--as-needed]} {target-winxp [/DLL]}]}}
    macro("libraryshr_linkopts", (
      {"default": ""},
      {"target-linux": ["-fPIC", "-ldl", "-Wl,--as-needed"]},
      {"target-winxp": "/DLL"},
    ))
    #### macro_append &{{libraryshr_linkopts [{default []} {target-linux [-Wl,--no-undefined]}]}}
    ctx.hwaf_macro_append("libraryshr_linkopts", (
      {"default": ""},
      {"target-linux": ["-Wl,--no-undefined"]},
    ))
    #### macro_append &{{libraryshr_linkopts [{default []} {target-linux [-pthread]}]}}
    ctx.hwaf_macro_append("libraryshr_linkopts", (
      {"default": ""},
      {"target-linux": "-pthread"},
    ))
    #### macro_append &{{libraryshr_linkopts [{default []} {target-cov [${linkdebugflags}]}]}}
    ctx.hwaf_macro_append("libraryshr_linkopts", (
      {"default": ""},
      {"target-cov": "${linkdebugflags}"},
    ))
    #### macro_append &{{libraryshr_linkopts [{default []} {host-x86_64&target-i386 [-m32]} {host-x86_64&target-i686 [-m32]} {host-darwin&target-i386 [-m32]} {host-i686&target-x86_64 [-m64]} {host-i386&target-x86_64 [-m64]}]}}
    ctx.hwaf_macro_append("libraryshr_linkopts", (
      {"default": ""},
      {("host-x86_64", "target-i386"): "-m32"},
      {("host-x86_64", "target-i686"): "-m32"},
      {("host-darwin", "target-i386"): "-m32"},
      {("host-i686", "target-x86_64"): "-m64"},
      {("host-i386", "target-x86_64"): "-m64"},
    ))
    #### macro_append &{{componentshr_linkopts [{default []} {target-mac []} {target-gcc&target-x86_64 [-Wl,-z,max-page-size=0x1000]}]}}
    ctx.hwaf_macro_append("componentshr_linkopts", (
      {"default": ""},
      {"target-mac": ""},
      {("target-gcc", "target-x86_64"): ["-Wl,-z,max-page-size=0x1000"]},
    ))
    #### macro_append &{{libraryshr_linkopts [{default []} {target-mac []} {target-gcc&target-x86_64 [-Wl,-z,max-page-size=0x1000]}]}}
    ctx.hwaf_macro_append("libraryshr_linkopts", (
      {"default": ""},
      {"target-mac": ""},
      {("target-gcc", "target-x86_64"): ["-Wl,-z,max-page-size=0x1000"]},
    ))
    #### macro &{{application_linkopts [{default []} {target-linux [-Wl,--export-dynamic]}]}}
    macro("application_linkopts", (
      {"default": ""},
      {"target-linux": ["-Wl,--export-dynamic"]},
    ))
    #### macro &{{shlibsuffix [{default [so]} {target-winxp [dll]}]}}
    macro("shlibsuffix", (
      {"default": "so"},
      {"target-winxp": "dll"},
    ))
    #### macro &{{shared_install_subdir [{default [/${tag}]} {target-winxp [\${tag}]} {use-shared-dir []}]}}
    macro("shared_install_subdir", (
      {"default": "/${tag}"},
      {"target-winxp": "\\${tag}"},
      {"use-shared-dir": ""},
    ))
    #### macro &{{genconfDir [{default [${shared_install_subdir}/genConf/]} {target-winxp [${shared_install_subdir}\genConf]}]}}
    macro("genconfDir", (
      {"default": "${shared_install_subdir}/genConf/"},
      {"target-winxp": "${shared_install_subdir}\\genConf"},
    ))
    ##### **** statement *hlib.MakeFragmentStmt (&{genconfig})
    #### macro &{{merge_genconfDb_cmd [{default [python ${GaudiPolicy_root}/scripts/merge_files.py]}]}}
    macro("merge_genconfDb_cmd", (
      {"default": ["python", "${GaudiPolicy_root}/scripts/merge_files.py"]},
    ))
    ##### **** statement *hlib.MakeFragmentStmt (&{merge_genconfDb})
    #### macro &{{genconfig_configurableModuleName [{default [GaudiKernel.Proxy]}]}}
    macro("genconfig_configurableModuleName", (
      {"default": "GaudiKernel.Proxy"},
    ))
    #### macro &{{genconfig_configurableDefaultName [{default [Configurable.DefaultName]}]}}
    macro("genconfig_configurableDefaultName", (
      {"default": "Configurable.DefaultName"},
    ))
    #### macro &{{genconfig_configurableAlgorithm [{default [ConfigurableAlgorithm]}]}}
    macro("genconfig_configurableAlgorithm", (
      {"default": "ConfigurableAlgorithm"},
    ))
    #### macro &{{genconfig_configurableAlgTool [{default [ConfigurableAlgTool]}]}}
    macro("genconfig_configurableAlgTool", (
      {"default": "ConfigurableAlgTool"},
    ))
    #### macro &{{genconfig_configurableAuditor [{default [ConfigurableAuditor]}]}}
    macro("genconfig_configurableAuditor", (
      {"default": "ConfigurableAuditor"},
    ))
    #### macro &{{genconfig_configurableService [{default [ConfigurableService]}]}}
    macro("genconfig_configurableService", (
      {"default": "ConfigurableService"},
    ))
    #### macro &{{genconfig_preload [{default []}]}}
    macro("genconfig_preload", (
      {"default": ""},
    ))
    ##### **** statement *hlib.PatternStmt (&{genconfig_add_preload macro_append genconfig_preload --load-library=<library> ;})
    ##### **** statement *hlib.PatternStmt (&{genconf_helper_lib library <package>GenConfHelperLib <src> ; macro <package>_genconfig_preload_lib $(<package>_root)/$(tag)/$(library_prefix)<package>GenConfHelperLib.$(shlibsuffix) target-winxp $(<package>_root)\$(tag)\$(library_prefix)<package>GenConfHelperLib.$(shlibsuffix) ; macro <package>GenConfHelperLib_shlibflags $(<package>GenConfHelperLib_use_linkopts) ; apply_pattern linker_library library=<package>GenConfHelperLib ; macro_remove <package>_linkopts -l<package>GenConfHelperLib target-winxp <package>GenConfHelperLib.lib ; macro_append <package>Conf_dependencies <package>GenConfHelperLib})
    ##### **** statement *hlib.PatternStmt (&{use_genconf_helper_lib private ; apply_pattern genconfig_add_preload library=$(<pkg>_genconfig_preload_lib) ; end_private})
    #### macro &{{use_GaudiCoreSvc [{default [GaudiCoreSvc * -no_auto_imports]} {no_genconf []} {do_genconf [GaudiCoreSvc * -no_auto_imports]} {ATLAS [GaudiCoreSvc * -no_auto_imports]}]}}
    macro("use_GaudiCoreSvc", (
      {"default": ["GaudiCoreSvc", "*", "-no_auto_imports"]},
      {"no_genconf": ""},
      {"do_genconf": ["GaudiCoreSvc", "*", "-no_auto_imports"]},
      {"ATLAS": ["GaudiCoreSvc", "*", "-no_auto_imports"]},
    ))
    ##### **** statement *hlib.PatternStmt (&{generate_configurables private ; use $(use_GaudiCoreSvc) apply_pattern generate_configurables_internal library=<library> group=<group> ; end_private})
    ##### **** statement *hlib.PatternStmt (&{generate_configurables_internal macro <package>_genConf_python  <project>_without_installarea&target-unix $(genconfInstallDir) <project>_without_installarea&target-winxp $(genconfInstallDir) ; path_remove PYTHONPATH ${<package>_genConf_python} ; path_prepend PYTHONPATH ${<package>_genConf_python} ; private ; macro run_genconfig_cmd do_real_genconfig no_genconf do_null_genconfig do_genconf do_real_genconfig ATLAS do_real_genconfig ; apply_pattern $(run_genconfig_cmd) library=<library> group=<group> ; end_private})
    ##### **** statement *hlib.PatternStmt (&{do_real_genconfig document genconfig <library>Conf -s=../$(tag) -group=<group> outdir=$(<package>_root)$(genconfDir)<package> library=<library> library_preload=$(genconfig_preload) conf_destdir=$(genconfInstallDir) confModuleName=$(genconfig_configurableModuleName) confDefaultName=$(genconfig_configurableDefaultName) confAlgorithm=$(genconfig_configurableAlgorithm) confAlgTool=$(genconfig_configurableAlgTool) confAuditor=$(genconfig_configurableAuditor) confService=$(genconfig_configurableService) $(library_prefix)<library>.$(shlibsuffix) ; macro_append <library>Conf_dependencies <library> ; apply_pattern install_python_init package=<package> group=<group> ; macro_append <group><package>_python_init_dependencies <library>Conf ; private ; macro merge_genconfDb_tag --do-merge do_merge_genconfDb --do-merge no_merge_genconfDb --un-merge ; document merge_genconfDb <library>ConfDbMerge -group=<group> inDir=$(<package>_root)$(genconfDir)<package> library=<library> merge_genconfDb_switch=$(merge_genconfDb_tag) $(bin)$(library_prefix)<library>.$(shlibsuffix) ; macro_append <library>ConfDbMerge_dependencies <library>Conf ; macro_append zip_<group><package>_python_modules_dependencies <library>ConfDbMerge ; end_private})
    ##### **** statement *hlib.PatternStmt (&{do_null_genconfig private ; macro dummy_for_<library>Conf <library> <group> ; end_private})
    ##### **** statement *hlib.PatternStmt (&{QMTestCommon private ; macro use_qmtest  QMTest QMtest * LCG_Interfaces -no_auto_imports ; use $(use_qmtest) ; macro use_pytools  QMTest pytools * LCG_Interfaces -no_auto_imports ; use $(use_pytools) ; set QMTESTRESULTS  QMTest ../../$(tag)/results.qmr ; end_private})
    ##### **** statement *hlib.PatternStmt (&{QMTest apply_pattern QMTestCommon ; private ; macro qmtest_local_dir ../tests/qmtest target-winxp ..\tests\qmtest ; set QMTESTLOCALDIR  QMTest $(qmtest_local_dir) ; action qmtest_run cmt -tag_add=QMTest run python $(GaudiPolicy_root)/scripts/run_qmtest.py $(package) $(cmt_args) ; macro_append qmtest_run_dependencies tests ; action TestPackage cmt qmtest_run $(cmt_args) ; action QMTestTestsDatabase cmt qmtest_run --no-output --dry-run ; action QMTestGUI cmt -tag_add=QMTest run cd $(qmtest_local_dir) ; qmtest gui' ; end_private})
    ##### **** statement *hlib.PatternStmt (&{QMTestSummarize apply_pattern QMTestCommon ; private ; macro qmtest_summarize_script $(GaudiPolicy_root)/scripts/qmtest_summarize.py ; action qmtest_summarize cmt -tag_add=QMTest run python $(qmtest_summarize_script) $(cmt_args) ; action TestProject cmt br cmt TestPackage ; cmt qmtest_summarize target-winxp cmt br cmt TestPackage & cmt qmtest_summarize ; end_private})
    #### path_append &{{QMTEST_CLASS_PATH [{default []} {QMTest [${GAUDIPOLICYROOT}/qmtest_classes]}]}}
    ctx.hwaf_path_append("QMTEST_CLASS_PATH", (
      {"default": ""},
      {"QMTest": "${GAUDIPOLICYROOT}/qmtest_classes"},
    ))
    #### macro_remove &{{cmt_actions_constituents [{default [make]}]}}
    ctx.hwaf_macro_remove("cmt_actions_constituents", (
      {"default": "make"},
    ))
    #### macro_remove &{{cmt_actions_constituents [{default [qmtest_run]}]}}
    ctx.hwaf_macro_remove("cmt_actions_constituents", (
      {"default": "qmtest_run"},
    ))
    #### macro_remove &{{cmt_actions_constituents [{default [qmtest_summarize]}]}}
    ctx.hwaf_macro_remove("cmt_actions_constituents", (
      {"default": "qmtest_summarize"},
    ))
    #### macro_remove &{{cmt_actions_constituents [{default [TestPackage]}]}}
    ctx.hwaf_macro_remove("cmt_actions_constituents", (
      {"default": "TestPackage"},
    ))
    #### macro_remove &{{cmt_actions_constituents [{default [TestProject]}]}}
    ctx.hwaf_macro_remove("cmt_actions_constituents", (
      {"default": "TestProject"},
    ))
    #### macro_remove &{{cmt_actions_constituents [{default [QMTestTestsDatabase]}]}}
    ctx.hwaf_macro_remove("cmt_actions_constituents", (
      {"default": "QMTestTestsDatabase"},
    ))
    #### macro_remove &{{cmt_actions_constituents [{default [QMTestGUI]}]}}
    ctx.hwaf_macro_remove("cmt_actions_constituents", (
      {"default": "QMTestGUI"},
    ))
    ##### **** statement *hlib.PatternStmt (&{disable_package_on private ; macro_remove_regexp all_constituents  <platform> .* ; macro_remove_regexp <package>_stamps  <platform> .* ; end_private})
    ##### **** statement *hlib.PatternStmt (&{GaudiApp use Gaudi * ; set GAUDIAPPNAME <package> ; set GAUDIAPPVERSION <version>})
    ##### **** statement *hlib.MakeFragmentStmt (&{call_command})
    ##### **** statement *hlib.PatternStmt (&{CallCommand document call_command <target> -group=<group> name="<target>})
    ##### **** statement *hlib.PatternStmt (&{NullCommand private ; macro dummy_for_NullCommand <target> <group> ; end_private})
    ##### **** statement *hlib.PatternStmt (&{ProjectVersionHeader private ; macro ProjectVersionHeader_output $(CMTINSTALLAREA)$(shared_install_subdir)/include/<project>_VERSION.h ; macro ProjectVersionHeader_deps ../cmt/requirements ; macro ProjectVersionHeader_command python $(GaudiPolicy_root)/scripts/createProjVersHeader.py <project> <version> $(ProjectVersionHeader_output) <project>_without_installarea  ; apply_pattern CallCommand target=ProjectVersionHeader ; end_private ; set <project>VERS <version>})
    ##### **** statement *hlib.PatternStmt (&{PackageVersionHeader private ; macro PackageVersionHeader_output <where>/<package>Version.h ; macro PackageVersionHeader_deps ../cmt/requirements ; macro PackageVersionHeader_command python $(GaudiPolicy_root)/scripts/createProjVersHeader.py <package> <version> $(PackageVersionHeader_output) <project>_without_installarea  ; apply_pattern CallCommand target=PackageVersionHeader ; end_private})
    ##### **** statement *hlib.MakeFragmentStmt (&{template})
    #### macro &{{strip_script [{default [${GAUDIPOLICYROOT}/scripts/StripPath]} {no-strip-path []} {target-winxp [${GAUDIPOLICYROOT}\scripts\StripPath]}]}}
    macro("strip_script", (
      {"default": "${GAUDIPOLICYROOT}/scripts/StripPath"},
      {"no-strip-path": ""},
      {"target-winxp": "${GAUDIPOLICYROOT}\\scripts\\StripPath"},
    ))
    #### macro &{{GenConfUser_script [{default [genconfuser.py]} {target-winxp [genconfuser.bat]}]}}
    macro("GenConfUser_script", (
      {"default": "genconfuser.py"},
      {"target-winxp": "genconfuser.bat"},
    ))
    ##### **** statement *hlib.PatternStmt (&{real_genconfuser private ; macro <package>GenConfUser_output $(<package>_root)$(genconfDir)<package>/<package>_user_confDb.py target-winxp $(<package>_root)$(genconfDir)<package>\<package>_user_confDb.py ; macro <package>GenConfUser_python $(<package>_root)/python target-winxp $(<package>_root)\python ; macro <package>GenConfUser_deps FORCE ; macro <package>GenConfUser_command $(GenConfUser_script) --lockerpath $(GaudiPolicy_root)/scripts -r $(<package>GenConfUser_python) -o $(<package>GenConfUser_output) <package> $(<package>ConfUserModules) ; apply_pattern CallCommand target=<package>GenConfUser ; document merge_genconfDb <package>ConfUserDbMerge -group=<group> inDir=$(<package>_root)$(genconfDir)<package> library=<package>_user merge_genconfDb_switch="--do-merge ; macro_append <package>_python_init_dependencies <package>GenConfUser ; macro_append <package>ConfUserDbMerge_dependencies <package>GenConfUser <package>_python ; end_private})
    ##### **** statement *hlib.PatternStmt (&{fake_genconfuser })
    ##### **** statement *hlib.PatternStmt (&{genconfuser private ; macro do_genconfuser real_genconfuser skip_genconfuser fake_genconfuser ; apply_pattern $(do_genconfuser) ; end_private})
    ##### **** statement *hlib.PatternStmt (&{container_package private ; apply_tag container-package ; end_private})
    ##### **** statement *hlib.PatternStmt (&{zip_python_modules private ; macro zip_<group><package>_python_modules_output $(CMTINSTALLAREA)$(shared_install_subdir)/python.zip ; macro zip_<group><package>_python_modules_deps FORCE ; macro zip_<group><package>_python_modules_command python $(GaudiPolicy_root)/scripts/ZipPythonDir.py $(CMTINSTALLAREA)$(shared_install_subdir)/python ; macro zip_<group><package>_python_modules_pattern_applied CallCommand no-pyzip&container-package CallCommand no-pyzip NullCommand ; apply_pattern $(zip_<group><package>_python_modules_pattern_applied) target=zip_<group><package>_python_modules group=<group> ; end_private})
    ##### **** statement *hlib.PatternStmt (&{zip_python_modules_container_fallback -global private ; macro zip_python_modules_container_fallback_pattern  no-pyzip&container-package zip_python_modules ; apply_pattern $(zip_python_modules_container_fallback_pattern) ; end_private})
    ##### **** statement *hlib.MakeFragmentStmt (&{install_resources})
    ##### **** statement *hlib.PatternStmt (&{doxy_vers_set set <external>VERS $(<external>_config_version)})
    ##### **** statement *hlib.SetStmt (&{{LCGCMTVERS [{default [${LCG_config_version}]}]}})
    ##### **** statement *hlib.SetStmt (&{{GAUDI_DOXY_HOME [{default [${GAUDI_home}/GaudiRelease/doc]}]}})
    #### tag &{ATLAS [no-pyzip]}
    ctx.hwaf_declare_tag(
        "ATLAS",
        content=["no-pyzip"]
    )
    #### macro_append &{{pp_cppflags [{default []} {ATLAS [-DATLAS_GAUDI_V21]}]}}
    ctx.hwaf_macro_append("DEFINES", (
      {"default": ""},
      {"ATLAS": "ATLAS_GAUDI_V21"},
    ))
    #### tag &{target-test [use-O3]}
    ctx.hwaf_declare_tag(
        "target-test",
        content=["use-O3"]
    )

    return # configure


### ---------------------------------------------------------------------------
def build(ctx):


    ##### **** statement *hlib.ApplyPatternStmt (&{doxy_vers_set [external=COOL]})
    ##### **** statement *hlib.ApplyPatternStmt (&{doxy_vers_set [external=CORAL]})
    ##### **** statement *hlib.ApplyPatternStmt (&{doxy_vers_set [external=ROOT]})
    ##### **** statement *hlib.ApplyPatternStmt (&{doxy_vers_set [external=Boost]})
    ##### **** statement *hlib.ApplyPatternStmt (&{doxy_vers_set [external=uuid]})
    ##### **** statement *hlib.ApplyPatternStmt (&{doxy_vers_set [external=GCCXML]})
    ##### **** statement *hlib.ApplyPatternStmt (&{doxy_vers_set [external=AIDA]})
    ##### **** statement *hlib.ApplyPatternStmt (&{doxy_vers_set [external=XercesC]})
    ##### **** statement *hlib.ApplyPatternStmt (&{doxy_vers_set [external=GSL]})
    ##### **** statement *hlib.ApplyPatternStmt (&{doxy_vers_set [external=Python]})
    ##### **** statement *hlib.ApplyPatternStmt (&{doxy_vers_set [external=HepMC]})
    ##### **** statement *hlib.ApplyPatternStmt (&{doxy_vers_set [external=QMtest]})


    return # build

## EOF ##
