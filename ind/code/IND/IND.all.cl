/* CodeCenter Project File */
/* Options for project */
unsetopt ansi
setopt auto_compile
unsetopt auto_reload
setopt auto_replace
unsetopt batch_load
unsetopt batch_run
unsetopt cc_prog
setopt ccargs               -g
unsetopt create_file
unsetopt debug_child
unsetopt echo
setopt edit_jobs            1
unsetopt eight_bit
unsetopt ignore_sharp_lines
setopt line_edit
setopt line_meta
setopt lint_load            2
setopt lint_run             2
setopt list_action
unsetopt load_flags
unsetopt long_not_int
unsetopt make_args
setopt make_hfiles
unsetopt make_offset
unsetopt make_prog
setopt make_symbol          #
setopt mem_config           16384
unsetopt mem_trace
setopt num_proc             1
unsetopt page_cmds
setopt page_list            15
unsetopt page_load
setopt path                 ~/IND/Treelib ~/IND/Statlib ~/IND/Eglib ~/IND/Trees ~/IND/Lib ~/IND/Include ~/IND/Graph
setopt proto_path           . /usr/local/saber_dir30/sun4-40/proto /usr/local/saber_dir30/sun4-40/../common/proto
unsetopt preprocessor
setopt program_name         a.out
unsetopt print_custom
setopt print_pointer
setopt print_string         20
unsetopt save_memory
setopt sbrk_size            1048576
setopt src_err              3
setopt src_step             1
setopt src_stop             3
unsetopt swap_uses_path
setopt sys_load_flags       -L/lib -L/usr/lib -L/usr/local/lib -I/usr/include -Dunix -Dsun -Dsparc -I~/IND/Include
unsetopt tab_stop
unsetopt terse_suppress
unsetopt terse_where
setopt unset_value          191
unsetopt win_fork_nodup
unsetopt win_no_raise
unsetopt win_message_list
unsetopt win_project_list
/* Suppressions for project */
suppress 5 
/* DEFAULT SUPPRESSION: Over/underflow unsigned <minus> */
suppress 7 
/* DEFAULT SUPPRESSION: Over/underflow unsigned <plus> */
suppress 25 symlex.c 
/* Initializing bad pointer */
suppress 26 symlex.c 
/* Storing bad pointer */
suppress 29 symlex.c 
/* Comparing bad pointer */
suppress 51 /usr/include/regexp.h 
/* DEFAULT SUPPRESSION: Information lost <assignment> */
suppress 52 
/* DEFAULT SUPPRESSION: Information lost <float> */
suppress 66 symyacc.c 
/* Too few function arguments */
suppress 68 on shuffle 
/* Benign argument mismatch */
suppress 68 on sfree 
/* Benign argument mismatch */
suppress 69 symyacc.c 
/* Serious argument mismatch */
suppress 97 in xfree 
/* DEFAULT SUPPRESSION: Freeing memory */
suppress 157 on __builtin_va_alist 
/* DEFAULT SUPPRESSION: Formal variable */
suppress 450 /usr/include/math.h 
/* DEFAULT SUPPRESSION: Float constant overflow */
suppress 460 /usr/include/ 
/* DEFAULT SUPPRESSION: Nested comment */
suppress 476 
/* DEFAULT SUPPRESSION: Extra text in directive */
suppress 495 on alloca 
/* DEFAULT SUPPRESSION: Too few macro arguments */
suppress 529 symlex.c 
/* Statement not reached */
suppress 530 
/* Empty body of statement */
suppress 560 
/* Assignment within conditional */
suppress 590 exp_nodes.c 
/* Unused formal parameter */
suppress 590 tested.c 
/* Unused formal parameter */
suppress 590 tree_class.c 
/* Unused formal parameter */
suppress 590 tree_add.c 
/* Unused formal parameter */
suppress 590 alpha_min.c 
/* Unused formal parameter */
suppress 590 alpha_list.c 
/* Unused formal parameter */
suppress 590 alpha_err.c 
/* Unused formal parameter */
suppress 591 symyacc.c 
/* Unused automatic variable */
suppress 592 print_tree.c 
/* Unused static */
suppress 592 symyacc.c 
/* Unused static */
suppress 592 symlex.c 
/* Unused static */
suppress 593 symyacc.c 
/* Unused label */
suppress 593 symlex.c 
/* Unused label */
suppress 594 tablestats.c 
/* Set but not used */
suppress 594 in tree_fcheck 
/* Set but not used */
suppress 595 test_display.c 
/* Used before set */
suppress 731 /lib/libsuntool.a(sel_common.o) 
/* DEFAULT SUPPRESSION: Text/data redefinition */
suppress 733 
/* DEFAULT SUPPRESSION: `static' formerly `extern' */
suppress 734 
/* DEFAULT SUPPRESSION: CenterLine function redefined */
suppress 760 
/* DEFAULT SUPPRESSION: Non-ANSI field syntax */
suppress 761 
/* DEFAULT SUPPRESSION: Result of ?: used as lvalue */
suppress 762 
/* DEFAULT SUPPRESSION: Result of cast used as lvalue */
suppress 112 on cxic

/* Contents of project */
load  /lib/libc.sa.1.7
load  /lib/libc.so.1.7
load  /lib/libc.a
load  TREE.c alpha_err.c alpha_list.c alpha_min.c copy_tree.c dec_prune.c depth_prune.c 
load  free_tree.c leaf_gain.c minerr_prune.c pess_prune.c print_tree.c prior.c read_chtr.c read_tree.c recnt_tree.c reerr_tree.c
load  subtree_prop.c tree.c tree_add.c tree_check.c tree_class.c tree_reclass.c tree_reprob.c 
load  tree_subw.c tree_smooth.c heap.c
load  treesize.c write_chtr.c write_tree.c Ztoprob.c best_dec.c beta.c calc_mse.c cumm_dvec.c digamma.c
load  ex.c logbeta.c logsum.c mmprint.c most_common.c normrandom.c   
load  restrat.c same_class.c shuffle.c stats2.c std_error.c utility.c lists.c partition.c read_eg.c
load  read_enc_eg.c read_enc_egs.c sort_set.c splits_lists.c symbol.c symlex.c 
load  symyacc.c table.c tablestats.c tested.c test_display.c test_mem.c 
load  test_outcome.c test_random.c write_eg.c write_enc_eg.c assign_unk.c shuffle_set.c test_query.c
load  libutil.a
load  /lib/libm.a
load  /lib/libl.a
load  rem_tree.c tgen.c CC_CV_grow.c CC_test_grow.c CC_tst_prune.c choose.c makeopttree.c tree_test.c
load  exp_nodes.c interact.c wall_grow.c interrupt.c randset.c
load  hashc.c sample.c print_sym.c tree_smpl.c random.o
load check_distn.c modify.c trans_graph.c graph_reprob.c path.c traverse.c dgraph_opts.c
load is_dgraph.c perform.c wander_graph.c mess_length.c stored_tree.c wander_tree.c mod_primitive.c   suggest_mods.c 
link
stop in error_LF

/* Signals caught and ignored */
catch HUP
catch INT
catch QUIT
catch ILL
catch TRAP
catch IOT
catch EMT
catch FPE
catch KILL
catch BUS
catch SEGV
catch SYS
catch PIPE
catch TERM
catch URG
catch STOP
catch TSTP
catch TTIN
catch TTOU
catch IO
catch XCPU
catch XFSZ
catch VTALRM
catch PROF
catch LOST
catch USR1
catch USR2
ignore ALRM
ignore CONT
ignore CHLD
ignore WINCH

/* Status of project */
