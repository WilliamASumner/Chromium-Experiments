#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <sys/time.h> // for timing

#include <sys/syscall.h> // pid syscall

#include "cpu_utils.h" // AFF IN SCOPE

#include <unistd.h>
#include <sys/types.h> // getpid()

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

namespace base {
	class TimeTicks {
	public: 
		int64_t us_;
	};
	class TimeDelta {
		int64_t delta_;
	};
}



namespace blink {
	class DocumentParser {
		virtual void PrepareToStopParsing();
	};

	typedef void (*doc_parsing_ptr)(DocumentParser*); // requires the "this" at least

	void DocumentParser::PrepareToStopParsing() {
		//printf("Preparing to Stop; pid = %ld\n",syscall(__NR_getpid));
		doc_parsing_ptr real_fcn = (doc_parsing_ptr)dlsym(RTLD_NEXT,"_ZN5blink14DocumentParser20PrepareToStopParsingEv"); // use mangled name
		if (real_fcn == NULL) {
			//printf("Error finding function 'PrepareToStopParsing'\n");
			exit(1);
		}
		// Run all big cores for the duration of this function
		//_set_affinity_big();
		real_fcn(this);
		//_set_affinity_all();
	}



	class HTMLDocumentParser { // leaving this out for now, focusing on Web Performance API
		void ResumeParsingAfterYield();
	};

	typedef void (*resume_parsing_ptr)(HTMLDocumentParser*); // requires the "this" at least

	void HTMLDocumentParser::ResumeParsingAfterYield() {
		//printf("Resuming Parsing; pid = %ld\n",syscall(__NR_getpid));
		resume_parsing_ptr real_fcn = (resume_parsing_ptr)dlsym(RTLD_NEXT,"_ZN5blink18HTMLDocumentParser23ResumeParsingAfterYieldEv"); // use mangled name
		if (real_fcn == NULL) {
			printf("Error finding function 'ResumeParsingAfterYield'\n");
			exit(1);
		}
		// Run all big cores for the duration of this function
		//_set_affinity_big();
		real_fcn(this);
		//_set_affinity_all();
	}

	/*class PaintTiming {
		void MarkFirstPaint();
	};

	void PaintTiming::MarkFirstPaint() {
		printf("First paint\n");
		resume_parsing_ptr real_fcn = (resume_parsing_ptr)dlsym(RTLD_NEXT,"_ZN5blink11PaintTiming14MarkFirstPaintEv"); // use mangled name
		if (real_fcn == NULL) {
			printf("Error finding function 'MarkFirstPaint'\n");
			exit(1);
		}
		// Run all big cores for the duration of this function
		//_set_affinity_big();
		real_fcn(this);
		//_set_affinity_all();
	}*/

	class DocumentLoadTiming {
	public:
		void SetNavigationStart(base::TimeTicks);
		void MarkNavigationStart();
	private:
		base::TimeTicks reference_monotonic_time_;
		base::TimeDelta reference_wall_time_;
		base::TimeTicks input_start_;
		base::TimeTicks navigation_start_;
		base::TimeTicks unload_event_start_;
		base::TimeTicks unload_event_end_;
		base::TimeTicks redirect_start_;
		base::TimeTicks redirect_end_;
		uint16_t redirect_count_;
	};

	typedef void (*set_nav_ptr)(DocumentLoadTiming*,base::TimeTicks); // requires the "this" at least
	void DocumentLoadTiming::SetNavigationStart(base::TimeTicks t) {
			printf("-----------Set Nav Start\n");
			printf("-----------old t = %ld\n",this->navigation_start_.us_);
			printf("-----------TimeTicks t = %ld\n",t.us_);
			printf("----------- redirect count %hu\n",this->redirect_count_);
			set_nav_ptr real_fcn = (set_nav_ptr)dlsym(RTLD_NEXT,"_ZN5blink18DocumentLoadTiming18SetNavigationStartEN4base9TimeTicksE"); // use mangled name
			if (real_fcn == NULL) {
					printf("Error finding function 'SetNavStart'\n");
					exit(1);
			}
			// Run all big cores for the duration of this function
			//_set_affinity_big();
			real_fcn(this,t);
			//_set_affinity_all();
	}

	typedef void (*mark_nav_ptr)(DocumentLoadTiming*); // requires the "this" at least
	void DocumentLoadTiming::MarkNavigationStart() {
			printf("=====*======Mark Nav Start\n");
			printf("============this->navigation_start_ = %ld\n",this->navigation_start_.us_);
			mark_nav_ptr real_fcn = (mark_nav_ptr)dlsym(RTLD_NEXT,"_ZN5blink18DocumentLoadTiming19MarkNavigationStartEv"); // use mangled name
			if (real_fcn == NULL) {
					printf("Error finding function 'MarkNavStart'\n");
					exit(1);
			}
			// Run all big cores for the duration of this function
			//_set_affinity_big();
			real_fcn(this);
			//_set_affinity_all();
	}
}
/*
	class WebPerformance {
		double NavigationStart() const;
		double FetchStart() const;
		double FirstPaint() const;
		double DomInteractive() const;
	};

	typedef double (*web_pref_ptr)(const WebPerformance*); // requires the "this" at least

	// NavigationTiming API
	double WebPerformance::NavigationStart() const {
		struct timeval t1,t2;
		int index = NAV_START;
		static web_pref_ptr real_fcn = (web_pref_ptr)dlsym(RTLD_NEXT,"_ZNK5blink14WebPerformance15NavigationStartEv"); // use mangled name

		if (real_fcn == NULL) {
			printf("Error finding function 'NavigationStart'\n");
			exit(1);
		}

		gettimeofday(&t1,NULL);
		double ret_val =  real_fcn(this);
		gettimeofday(&t2,NULL);
		
		process_avg_mtx.lock();

		call_count[index]++;
		moving_avg_dur[index] = ((double) (t2.tv_usec - t1.tv_usec) / 1000 +
						(double) (t2.tv_sec - t1.tv_sec) * 1000) + (call_count[index]-1) * moving_avg_dur[index];
		
		if(call_count[index] != 0) {
				moving_avg_dur[index] /= call_count[index];
		}

		process_avg_mtx.unlock();

		pid_t mypid = getpid();
		if (call_count[index] % 50 == 0) {
			printf("process (%d,'NavigationStart'): number of calls: %ld, Average Duration: %lf ms\n",mypid,call_count[index],moving_avg_dur[index]);
		}
		
		return ret_val;
	}

	double WebPerformance::FetchStart() const {
		struct timeval t1,t2;
		int index = FETCH_START;

		static web_pref_ptr real_fcn = (web_pref_ptr)dlsym(RTLD_NEXT,"_ZNK5blink14WebPerformance10FetchStartEv"); // use mangled name
		if (real_fcn == NULL) {
			printf("Error finding function 'FetchStart'\n");
			exit(1);
		}

		gettimeofday(&t1,NULL);
		double ret_val =  real_fcn(this);
		gettimeofday(&t2,NULL);
		
		process_avg_mtx.lock();

		call_count[index]++;
		moving_avg_dur[index] = ((double) (t2.tv_usec - t1.tv_usec) / 1000 +
						(double) (t2.tv_sec - t1.tv_sec) * 1000) + (call_count[index]-1) * moving_avg_dur[index];

		if (call_count[index] != 0) {
				moving_avg_dur[index] /= call_count[index];
		}

		process_avg_mtx.unlock();

		pid_t mypid = getpid();
		if (call_count[index] % 50 == 0) {
			printf("process (%d,'FetchStart'): number of calls: %ld, Average Duration: %lf ms\n",mypid,call_count[index],moving_avg_dur[index]);
		}
		
		return ret_val;
	}

	double WebPerformance::FirstPaint() const {
		struct timeval t1,t2;
		int index = FIRST_PAINT;

		static web_pref_ptr real_fcn = (web_pref_ptr)dlsym(RTLD_NEXT,"_ZNK5blink14WebPerformance10FirstPaintEv"); // use mangled name
		if (real_fcn == NULL) {
			printf("Error finding function 'FirstPaint'\n");
			exit(1);
		}

		gettimeofday(&t1,NULL);
		double ret_val =  real_fcn(this);
		gettimeofday(&t2,NULL);
		

		process_avg_mtx.lock();

		call_count[index]++;
		moving_avg_dur[index] = ((double) (t2.tv_usec - t1.tv_usec) / 1000 + 
						(double) (t2.tv_sec - t1.tv_sec) * 1000) + (call_count[index]-1) * moving_avg_dur[index];
		if (call_count[index] != 0) {
				moving_avg_dur[index] /= call_count[index];
		}

		process_avg_mtx.unlock();

		pid_t mypid = getpid();
		if (call_count[index] % 50 == 0) {
			printf("process (%d,'FirstPaint'): number of calls: %ld, Average Duration: %lf ms\n",mypid,call_count[index],moving_avg_dur[index]);
		}
		return ret_val;
	}

	double WebPerformance::DomInteractive() const {
			struct timeval t1,t2;
			int index = DOM_INTER;

			static web_pref_ptr real_fcn = (web_pref_ptr)dlsym(RTLD_NEXT,"_ZNK5blink14WebPerformance14DomInteractiveEv"); // use mangled name
			if (real_fcn == NULL) {
					printf("Error finding function 'DomInteractive'\n");
					exit(1);
			}

			gettimeofday(&t1,NULL);
			double ret_val =  real_fcn(this);
			gettimeofday(&t2,NULL);


			process_avg_mtx.lock();

			call_count[index]++;
			moving_avg_dur[index] = ((double) (t2.tv_usec - t1.tv_usec) / 1000 + 
							(double) (t2.tv_sec - t1.tv_sec) * 1000) + (call_count[index]-1) * moving_avg_dur[index];
			if (call_count[index] != 0) {
					moving_avg_dur[index] /= call_count[index];
			}

			process_avg_mtx.unlock();

			pid_t mypid = getpid();
			if (call_count[index] != 0 ) {
					printf("process (%d,'DomInteractive'): number of calls: %ld, Average Duration: %lf ms\n",mypid,call_count[index],moving_avg_dur[index]);
			}
			return ret_val;
	}
} */
