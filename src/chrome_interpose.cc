#include <dlfcn.h> // symbol lookup
#include "experiment/interpose.hh" // INTERPOSE
#include "experiment/experimenter.hh" // experiment fns
#include <stdio.h>

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "chrome_includes/v8/v8.h" // v8 stuff


/* Chrome Startup to initialize experimentt */
typedef int  (*main_fcn)(int, char**, char**);
typedef int  (*libc_main_fcn)(main_fcn,int,char**,void (*)(void), void(*)(void), void(*)(void),void*);
typedef void (*exit_fcn)(int);

thread_local main_fcn orig_main;
//int pgrp = 0;

int my_main(int argc, char **argv, char **env) {

   if (argv != nullptr && argc > 1 && argv[1] != nullptr) {
       if (strncmp(argv[1],"--type=renderer",15) == 0) { // renderer process
           experiment_init(argv[0]); // set up logger, register handlers
       } else if (strncmp(argv[1],"--no-zygote",12) == 0) { // initial process
           experiment_init(argv[0]);
           experiment_start_timer(); // just start a timer
           //pgrp = getpgrp(); // get the process group so we can kill all spawned processes later
       }
   }
   //fprintf(stderr,"\n\n\nProcess: %s has pgrp: %d and pid: %d\n\n\n",argv[1],getpgrp(),getpid());

   int result = orig_main(argc,argv,env);
   return result;
}


extern "C" int __libc_start_main(main_fcn main, int argc, char **ubp_av, void (*init)(void), void(*fini)(void), void (*rtld_fini) (void), void (*stack_end)) {

    libc_main_fcn start_main = (libc_main_fcn)dlsym(RTLD_NEXT,"__libc_start_main");
    if(start_main == NULL) {
        fprintf(stderr,"Error: no __libc_start_main found\n");
        exit(1);
    }
    orig_main = main;
    return  start_main(my_main,argc,ubp_av,init,fini,rtld_fini,stack_end); // Call real __libc_start_main
}

/* Because g3log only flushes on crashes, we need to tell it to flush on exit if a process exits normally */
extern "C" void _exit(int status) { //like exit but does not call onexit functions
    exit_fcn orig__exit = (exit_fcn)dlsym(RTLD_NEXT,"_exit");
    if(orig__exit == NULL) {
        fprintf(stderr,"Error: no _exit found\n");
        exit(1);
    }
    experiment_stop();
    orig__exit(status);
    abort();
}

extern "C" void exit(int status) { //like exit but does not call onexit functions
    exit_fcn orig_exit = (exit_fcn)dlsym(RTLD_NEXT,"exit");
    if(orig_exit == NULL) {
        fprintf(stderr,"Error: no _exit found\n");
        exit(1);
    }
    experiment_stop();
    orig_exit(status);
    abort();
}

/* WTF::String - used a lot in blink */
namespace WTF {
    class String;
};

namespace blink {

    /* HTML Stage */

    class HTMLDocumentParser { // leaving this out for now, focusing on Web Performance API
        void PumpPendingSpeculations();
        void ResumeParsingAfterYield();
    };

    typedef void (*pump_pend_ptr)(HTMLDocumentParser*); // requires the "this" at least

    void HTMLDocumentParser::PumpPendingSpeculations() {

        pump_pend_ptr real_fcn =
            (pump_pend_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink18HTMLDocumentParser23PumpPendingSpeculationsEv"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'PumpPendingSpeculations'\n");
            exit(1);
        }

        experiment_fentry("PumpPendingSpeculations");
        real_fcn(this);
        experiment_fexit("PumpPendingSpeculations");
    }


    typedef void (*resume_parse_ptr)(HTMLDocumentParser*); // requires the "this" at least

    void HTMLDocumentParser::ResumeParsingAfterYield() {
        resume_parse_ptr real_fcn =
            (resume_parse_ptr)dlsym(RTLD_NEXT,"_ZN5blink18HTMLDocumentParser23ResumeParsingAfterYieldEv"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'ResumeParsingAfterYield'\n");
            exit(1);
        }

        experiment_fentry("ResumeParsingAfterYield");
        real_fcn(this);
        experiment_fexit("ResumeParsingAfterYield");
    }

    /* Requisite Class Definitions */

    enum class ParseSheetResult; // css_parser.h
    class CSSParserContext;
    class StyleSheetContents;
    enum class CSSDeferPropertyParsing; // css_parser_mode.h
        class CSSParser {
        static ParseSheetResult ParseSheet(
                const CSSParserContext*,
                StyleSheetContents*,
                const WTF::String&,
                CSSDeferPropertyParsing defer_property_parsing,
                bool);
    };

    /* Function Pointer Definitions */

    typedef ParseSheetResult (*parse_sheet_ptr)(
            const CSSParserContext*,
            StyleSheetContents*, 
            const WTF::String&,
            CSSDeferPropertyParsing,
            bool); // static func no this

    /* Function re-definition */

    ParseSheetResult CSSParser::ParseSheet(
            const CSSParserContext* context,
            StyleSheetContents* style_sheet,
            const WTF::String& text,
            CSSDeferPropertyParsing defer_property_parsing,
            bool allow_import_rules) {


        parse_sheet_ptr real_fcn =
            (parse_sheet_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink9CSSParser10ParseSheetEPKNS_16CSSParserContextEPNS_18StyleSheetContentsERKN3WTF6StringENS_23CSSDeferPropertyParsingEb"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'ParseSheet'\n");
            exit(1);
        }

        // enum class, should have no self param
        experiment_fentry("ParseSheet");
        ParseSheetResult result = real_fcn(context,style_sheet,text,defer_property_parsing,allow_import_rules);
        experiment_fexit("ParseSheet");

        return result;
    }



    class Document {
        public:
            void UpdateStyleAndLayoutTree();

            enum DocumentReadyState { kLoading, kInteractive, kComplete };
            void SetReadyState(DocumentReadyState);
    };

    typedef void (*update_style_ptr)(Document*); // static func no this


    void Document::UpdateStyleAndLayoutTree() {

        update_style_ptr real_fcn =
            (update_style_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink8Document24UpdateStyleAndLayoutTreeEv");
        if (real_fcn == NULL) {
            printf("Error finding function 'UpdateStyleAndLayoutTree'\n");
            exit(1);
        }

        experiment_fentry("UpdateStyleAndLayoutTree");
        real_fcn(this);
        experiment_fexit("UpdateStyleAndLayoutTree");
    }

    /* This function is purely for monitoring the progress of the page load */
    typedef void(*update_rdy_ptr)(Document*,Document::DocumentReadyState);

    void Document::SetReadyState(Document::DocumentReadyState ready_state) {
        update_rdy_ptr real_fcn =
            (update_rdy_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink8Document13SetReadyStateENS0_18DocumentReadyStateE");
        if (real_fcn == NULL) {
            printf("Error finding function 'SetReadyState'\n");
            exit(1);
        }

        switch(ready_state) {
            case kInteractive:
                break;
            case kLoading:
                break; // still loading
            case kComplete:
                experiment_mark_page_loaded(); // js now running
                break; // full page load
            default:
                break;
        }
        real_fcn(this,ready_state);
    }

    /* Paint/Layout Stage */
    class DocumentLifecycle {
        public:
            enum LifecycleState {
                kUninitialized,
                kInactive,

                // When the document is active, it traverses these states.

                kVisualUpdatePending,

                kInStyleRecalc,
                kStyleClean,

                kInLayoutSubtreeChange,
                kLayoutSubtreeChangeClean,

                kInPreLayout,
                kInPerformLayout,
                kAfterPerformLayout,
                kLayoutClean,

                kInCompositingUpdate,
                kCompositingInputsClean,
                kCompositingClean,

                // In InPrePaint step, any data needed by painting are prepared.
                // Paint property trees are built and paint invalidations are issued.
                kInPrePaint,
                kPrePaintClean,

                // In InPaint step, paint artifacts are generated and raster invalidations
                // are issued.
                // In CAP, composited layers are generated/updated.
                kInPaint,
                kPaintClean,

                // Once the document starts shutting down, we cannot return
                // to the style/layout/compositing states.
                kStopping,
                kStopped,
            };
    };

    class LocalFrameView {
        void UpdateLifecyclePhasesInternal(
                DocumentLifecycle::LifecycleState target_state);
        void PerformLayout(bool);
    };

    /* Paint Stage */

    typedef void (*update_lifecycle_ptr)(
            LocalFrameView*,
            DocumentLifecycle::LifecycleState);

    void LocalFrameView::UpdateLifecyclePhasesInternal(
            DocumentLifecycle::LifecycleState target_state) {

        update_lifecycle_ptr real_fcn =
            (update_lifecycle_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink14LocalFrameView29UpdateLifecyclePhasesInternalENS_17DocumentLifecycle14LifecycleStateE"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'UpdateLifecyclePhasesInternal'\n");
            exit(1);
        }

        experiment_fentry("UpdateLifecyclePhasesInternal");
        real_fcn(this,target_state);
        experiment_fexit("UpdateLifecyclePhasesInternal");
    }

    /* Layout Stage */

    typedef void (*perform_layout_ptr)(
            LocalFrameView*,
            bool);
    void LocalFrameView::PerformLayout(bool in_subtree_layout) {

        perform_layout_ptr real_fcn =
            (perform_layout_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink14LocalFrameView13PerformLayoutEb"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'PerformLayout'\n");
            exit(1);
        }

        experiment_fentry("PerformLayout");
        real_fcn(this,in_subtree_layout);
        experiment_fexit("PerformLayout");

    }


    /* JS Stage */

    class KURL;
    class ScriptSourceCode;
    enum class SanitizeScriptErrors;
    class ScriptFetchOptions;

    class ScriptController {
        void ExecuteScriptInMainWorld(
                const ScriptSourceCode&,
                const KURL& base_url,
                SanitizeScriptErrors,
                const ScriptFetchOptions&);
        v8::Local<v8::Value> ExecuteScriptInIsolatedWorld(
                int32_t world_id,
                const ScriptSourceCode& source,
                const KURL& base_url,
                SanitizeScriptErrors sanitize_script_errors);

    };

    typedef void (*execute_script_ptr)(
            ScriptController*, 
            const ScriptSourceCode&,
            const KURL& base_url,
            SanitizeScriptErrors,
            const ScriptFetchOptions&);

    void ScriptController::ExecuteScriptInMainWorld(
            const ScriptSourceCode& source_code,
            const KURL& base_url,
            SanitizeScriptErrors sanitize_script_errors,
            const ScriptFetchOptions& fetch_options){


        execute_script_ptr real_fcn =
            (execute_script_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink16ScriptController24ExecuteScriptInMainWorldERKNS_16ScriptSourceCodeERKNS_4KURLENS_20SanitizeScriptErrorsERKNS_18ScriptFetchOptionsE"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'ExecuteScriptInMainWorld'\n");
            exit(1);
        }

        experiment_fentry("ExecuteScriptInMainWorld");
        real_fcn(this,source_code,base_url,sanitize_script_errors,fetch_options);
        experiment_fexit("ExecuteScriptInMainWorld");
    }



    typedef v8::Local<v8::Value> (*execute_script_isl_ptr)(
            ScriptController*, 
            int32_t world_id,
            const ScriptSourceCode&,
            const KURL&,
            SanitizeScriptErrors);

     /* To test this function: install adblockplus and visit ads-blocker.com. This function
      * only executes in an extension and not from page javascript. SEE:
      * ~/chromium/src/third_party/blink/renderer/bindings/core/v8/V8BindingDesign.md*/
    
    v8::Local<v8::Value> ScriptController::ExecuteScriptInIsolatedWorld(
            int32_t world_id,
            const ScriptSourceCode& source,
            const KURL& base_url,
            SanitizeScriptErrors sanitize_script_errors) {

        execute_script_isl_ptr real_fcn =
            (execute_script_isl_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink16ScriptController28ExecuteScriptInIsolatedWorldEiRKNS_16ScriptSourceCodeERKNS_4KURLENS_20SanitizeScriptErrorsE"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'ExecuteScriptInIsolatedWorld'\n");
            exit(1);
        }

        experiment_fentry("ExecuteScriptInIsolatedWorld");
        v8::Local<v8::Value> result = real_fcn(this,world_id,source,base_url,sanitize_script_errors);
        experiment_fexit("ExecuteScriptInIsolatedWorld");

        return result;
    }


    class ExecutionContext;

    class V8ScriptRunner {
        static v8::MaybeLocal<v8::Value> CallFunction(
                v8::Local<v8::Function>,
                ExecutionContext*,
                v8::Local<v8::Value>,
                int,
                v8::Local<v8::Value> args[],
                v8::Isolate*);
    };

    typedef v8::MaybeLocal<v8::Value> (*script_runner_ptr)(
            v8::Local<v8::Function>,
            ExecutionContext*,
            v8::Local<v8::Value>,
            int,
            v8::Local<v8::Value>*,
            v8::Isolate*);


    v8::MaybeLocal<v8::Value> V8ScriptRunner::CallFunction(
            v8::Local<v8::Function> function,
            ExecutionContext* context,
            v8::Local<v8::Value> receiver,
            int argc,
            v8::Local<v8::Value> args[],
            v8::Isolate* isolate) {

        script_runner_ptr real_fcn =
            (script_runner_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink14V8ScriptRunner12CallFunctionEN2v85LocalINS1_8FunctionEEEPNS_16ExecutionContextENS2_INS1_5ValueEEEiPS8_PNS1_7IsolateE"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'CallFunction'\n");
            exit(1);
        }

        experiment_fentry("CallFunction");
        v8::MaybeLocal<v8::Value> result = real_fcn(function,context,receiver,argc,args,isolate);
        experiment_fexit("CallFunction");
        return result;

    }

}
