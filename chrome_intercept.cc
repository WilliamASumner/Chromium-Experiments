#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <sys/time.h> // for timing

#include <sys/syscall.h> // pid syscall

#include "cpu_utils.h" // AFF IN SCOPE

#include <unistd.h>
#include <sys/types.h> // getpid()

#include "chrome_includes/v8/v8.h" // v8 stuff

#include <mutex>
std::mutex process_avg_mtx;

/* Stat buffers */
long call_count[30] = {0};
double moving_avg_dur[30] = {0.0};
double moving_avg_len[30] = {0.0};

/* Defines for interposed function indices into stat buffers */
#define NAV_START   0
#define FETCH_START 1
#define FIRST_PAINT 2
#define DOM_INTER   3


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
        //printf("\n\n\n");
        //printf("PumpPendingSpeculations; tid = %lu\n",syscall(SYS_gettid));
        //printf("\n\n\n");

        pump_pend_ptr real_fcn =
            (pump_pend_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink18HTMLDocumentParser23PumpPendingSpeculationsEv"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'PumpPendingSpeculations'\n");
            exit(1);
        }

        real_fcn(this);
    }


    typedef void (*resume_parse_ptr)(HTMLDocumentParser*); // requires the "this" at least

    void HTMLDocumentParser::ResumeParsingAfterYield() {
        //printf("Resuming Parsing; tid = %lu\n",syscall(SYS_gettid));
        resume_parse_ptr real_fcn =
            (resume_parse_ptr)dlsym(RTLD_NEXT,"_ZN5blink18HTMLDocumentParser23ResumeParsingAfterYieldEv"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'ResumeParsingAfterYield'\n");
            exit(1);
        }
        // Run all big cores for the duration of this function
        //_set_affinity_big();
        real_fcn(this);
        //_set_affinity_all();
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

        //printf("\n\n\n");
        //printf("ParseSheet; tid = %lu\n",syscall(SYS_gettid));
        //printf("\n\n\n");

        parse_sheet_ptr real_fcn =
            (parse_sheet_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink9CSSParser10ParseSheetEPKNS_16CSSParserContextEPNS_18StyleSheetContentsERKN3WTF6StringENS_23CSSDeferPropertyParsingEb"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'ParseSheet'\n");
            exit(1);
        }

        // enum class, should have no self param
        ParseSheetResult result = real_fcn(context,style_sheet,text,defer_property_parsing,allow_import_rules);

        return result;
    }


    class Document {
        void UpdateStyleAndLayoutTree();
    };

    typedef void (*update_style_ptr)(Document*); // static func no this


    void Document::UpdateStyleAndLayoutTree() {
        //printf("\n\n\n");
        //printf("UpdateStyle; tid = %lu\n",syscall(SYS_gettid));
        //printf("\n\n\n");

        update_style_ptr real_fcn =
            (update_style_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink8Document24UpdateStyleAndLayoutTreeEv"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'UpdateStyleAndLayoutTree'\n");
            exit(1);
        }

        real_fcn(this);
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
        //printf("\n\n\n");
        //printf("UpdateLifecycle;tid= %lu\n",syscall(SYS_gettid));
        //printf("\n\n\n");

        update_lifecycle_ptr real_fcn =
            (update_lifecycle_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink14LocalFrameView29UpdateLifecyclePhasesInternalENS_17DocumentLifecycle14LifecycleStateE"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'UpdateLifecyclePhasesInternal'\n");
            exit(1);
        }

        real_fcn(this,target_state);
    }

    /* Layout Stage */

    typedef void (*perform_layout_ptr)(
            LocalFrameView*,
            bool);
    void LocalFrameView::PerformLayout(bool in_subtree_layout) {
        //printf("\n\n\n");
        //printf("PerformLayout;tid= %lu\n",syscall(SYS_gettid));
        //printf("\n\n\n");

        perform_layout_ptr real_fcn =
            (perform_layout_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink14LocalFrameView13PerformLayoutEb"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'PerformLayout'\n");
            exit(1);
        }

        real_fcn(this,in_subtree_layout);

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

        //printf("\n\n\n");
        //printf("execute script in main;tid= %lu\n",syscall(SYS_gettid));
        //printf("\n\n\n");

        execute_script_ptr real_fcn =
            (execute_script_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink16ScriptController24ExecuteScriptInMainWorldERKNS_16ScriptSourceCodeERKNS_4KURLENS_20SanitizeScriptErrorsERKNS_18ScriptFetchOptionsE"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'ExecuteScriptInMainWorld'\n");
            exit(1);
        }

        real_fcn(this,source_code,base_url,sanitize_script_errors,fetch_options);
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
        printf("\n\n\n");
        printf("execute script in isolated;tid= %lu\n",syscall(SYS_gettid));
        printf("\n\n\n");

        execute_script_isl_ptr real_fcn =
            (execute_script_isl_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink16ScriptController28ExecuteScriptInIsolatedWorldEiRKNS_16ScriptSourceCodeERKNS_4KURLENS_20SanitizeScriptErrorsE"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'ExecuteScriptInIsolatedWorld'\n");
            exit(1);
        }

        return real_fcn(this,world_id,source,base_url,sanitize_script_errors);

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

        //printf("\n\n\n");
        //printf("CallFunction;tid= %lu\n",syscall(SYS_gettid));
        //printf("\n\n\n");

        script_runner_ptr real_fcn =
            (script_runner_ptr)dlsym(RTLD_NEXT,
                    "_ZN5blink14V8ScriptRunner12CallFunctionEN2v85LocalINS1_8FunctionEEEPNS_16ExecutionContextENS2_INS1_5ValueEEEiPS8_PNS1_7IsolateE"); // use mangled name
        if (real_fcn == NULL) {
            printf("Error finding function 'CallFunction'\n");
            exit(1);
        }

        return real_fcn(function,context,receiver,argc,args,isolate);

    }

}
