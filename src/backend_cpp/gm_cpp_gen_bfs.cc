
#include <stdio.h>
#include "gm_backend_cpp.h"
#include "gm_error.h"
#include "gm_code_writer.h"
#include "gm_frontend.h"

// TODO
void gm_cpp_gen::generate_bfs_body_fw(ast_bfs* bfs)
{
    // filters should have been changed into if
    ast_sentblock *sb = bfs->get_fbody();
    assert(sb!=NULL);
    generate_sent_block(sb, false);
}

void gm_cpp_gen::generate_bfs_body_bw(ast_bfs* bfs)
{
    ast_sentblock *sb = bfs->get_bbody();
    assert(sb!=NULL);
    generate_sent_block(sb, false);
}

void gm_cpp_gen::generate_bfs_def(ast_bfs* bfs) 
{
    const char* bfs_name = bfs->find_info_string(CPPBE_INFO_BFS_NAME);
    const char* level_t = "short";
    const char* use_multithread = is_target_omp() ? "true" : "false";
    const char* save_child = "false"; // [XXX]
    const char* use_reverse_edge = bfs->is_transpose() ? "true" : "false";
    const char* has_navigator = (bfs->get_navigator()==NULL) ? "false" : "true";

    ast_extra_info_set* info = (ast_extra_info_set*)  bfs->find_info(CPPBE_INFO_BFS_SYMBOLS);
    std::set<void*>& SET = info->get_set();
    std::set<void*>::iterator S;
    gm_symtab_entry* graph_sym = (gm_symtab_entry*) (*(SET.begin()));

    sprintf(temp, "class %s : public %s", bfs_name, BFS_TEMPLATE);
    Body.pushln(temp);
    Body.push_indent();
    sprintf(temp, "<%s, %s, %s, %s, %s>",
            level_t, use_multithread, has_navigator, use_reverse_edge, save_child ); 
    Body.pushln(temp);
    Body.pop_indent();
    Body.pushln("{");

    Body.pop_indent();
    Body.pushln("public:");
    Body.push_indent();

    sprintf(temp, "%s(", bfs_name);
    Body.push(temp);

    //------------------------------------------
    // constructor
    //------------------------------------------
    bool is_first = true;
    int total = SET.size();
    int i =0;
    int NL = 3;
    for(S=SET.begin(); S!=SET.end(); S++, i++) {
        if (!is_first) 
            Body.push(", ");
        if ((i>0) && (i != total) && ((i%NL)==0)) Body.NL();
        is_first = false;
        gm_symtab_entry* sym = (gm_symtab_entry*)(*S);
        ast_typedecl* t = sym->getType();
        Body.push(get_type_string(t));
        Body.push("&");
        Body.push(" _");
        Body.push(sym->getId()->get_genname());
    }
    Body.pushln(")");
    sprintf(temp, ": %s", BFS_TEMPLATE);
    Body.push(temp);
    sprintf(temp, "<%s, %s, %s, %s, %s>(%s),",
            level_t, use_multithread, has_navigator, use_reverse_edge, save_child,
            graph_sym->getId()->get_genname()); 
    Body.pushln(temp);

    // init list
    is_first = true;
    NL = 6;
    i = 0;
    for(S=SET.begin(); S!=SET.end(); S++, i++) {
        if (!is_first) Body.push(", ");
        if ((i>0) && (i != total) && ((i%NL)==0)) Body.NL();
        is_first = false;
        gm_symtab_entry* sym = (gm_symtab_entry*)(*S);
        Body.push(sym->getId()->get_genname());
        Body.push('(');
        Body.push('_');
        Body.push(sym->getId()->get_genname());
        Body.push(')');
    }
    Body.pushln("{}");
    Body.NL();

    //-------------------------------------------------
    // list of scope variables
    //-------------------------------------------------
    Body.pop_indent();
    Body.pushln("private:  // list of varaibles");
    Body.push_indent();
    for(S=SET.begin(); S!=SET.end(); S++, i++) {
        gm_symtab_entry* sym = (gm_symtab_entry*)(*S);
        ast_typedecl* t = sym->getType();
        Body.push(get_type_string(t));
        Body.push("& ");
        Body.push(sym->getId()->get_genname());
        Body.pushln(";");
    }
    Body.NL();

    Body.pop_indent();
    Body.pushln("protected:");
    Body.push_indent();

    if (bfs->get_fbody() == NULL) {
        Body.pushln("virtual void visit_fw() {}");
    }
    else {
        Body.pushln("virtual void visit_fw()");
        generate_bfs_body_fw(bfs);
        Body.NL();
    }

    if (bfs->get_fbody() == NULL) {
        Body.pushln("virtual void visit_rv() {}");
    }
    else {
        Body.pushln("virtual void visit_rv()");
        generate_bfs_body_bw(bfs);
        Body.NL();
    }

    generate_bfs_body_bw(bfs);
    Body.NL();

    Body.pushln("virtual bool check_navigator() {return true;}");
    Body.NL();

    Body.NL();
    Body.pushln("};");
    Body.NL();
}


void gm_cpp_gen::generate_sent_bfs(ast_bfs* bfs)
{
    /*
    generate_bfs_init(bfs);

    generate_bfs_main(bfs);

    generate_bfs_main_back(bfs);

    generate_bfs_finish(bfs);
    */

    //-------------------------------------------
    // (1) create BFS object
    //-------------------------------------------
    const char* bfs_name = bfs->find_info_string(CPPBE_INFO_BFS_NAME);
    const char* bfs_inst_name = FE.voca_temp_name_and_add("_BFS", "");
    sprintf(temp, "%s %s", bfs_name, bfs_inst_name); 
    Body.push(temp);
    Body.push('(');
    //-------------------------------------------
    // give every entry that are used
    //-------------------------------------------
    ast_extra_info_set* syms = (ast_extra_info_set*) bfs->find_info(CPPBE_INFO_BFS_SYMBOLS);
    assert(syms != NULL);
    std::set<void*>& S  = syms->get_set();
    std::set<void*>::iterator I;
    bool is_first = true;
    for (I = S.begin(); I!=S.end(); I++)
    {
        if (!is_first) {Body.push(", ");}
        is_first = false;
        gm_symtab_entry* e = (gm_symtab_entry*) *I; 
        Body.push( e->getId()->get_genname());
    }
    Body.pushln(");");

    //-------------------------------------------
    // (2) Make a call to it
    //-------------------------------------------
    Body.push(bfs_inst_name);
    Body.push(".");
    Body.push(DO_BFS_FORWARD);
    Body.pushln("();");

    if (bfs->get_bbody() != NULL)
    {
        Body.push(bfs_inst_name);
        Body.push(".");
        Body.push(DO_BFS_REVERSE);
        Body.pushln("();");
    }

    delete [] bfs_inst_name;
    return; 
}