#ifndef __GAMERUN_H
#define __GAMERUN_H

//#include "Headers.hpp"
#include "PCQueue.hpp"
#include "Thread.hpp"
#include "utils.hpp"

/*--------------------------------------------------------------------------------
								  Species colors
--------------------------------------------------------------------------------*/
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black - 7 */
#define RED     "\033[31m"      /* Red - 1*/
#define GREEN   "\033[32m"      /* Green - 2*/
#define YELLOW  "\033[33m"      /* Yellow - 3*/
#define BLUE    "\033[34m"      /* Blue - 4*/
#define MAGENTA "\033[35m"      /* Magenta - 5*/
#define CYAN    "\033[36m"      /* Cyan - 6*/


/*--------------------------------------------------------------------------------
								  Auxiliary Structures
--------------------------------------------------------------------------------*/

struct game_params {
	// All here are derived from ARGV, the program's input parameters.
	uint n_gen;
	uint n_thread;
	string filename;
	bool interactive_on;
	bool print_on;
};

struct job_tile{
	int first_row;
	int last_row;
	int current_phase;
	vector<vector<int>> *curr;
	vector<vector<int>> *next;
};
/*--------------------------------------------------------------------------------
									Class Declaration
--------------------------------------------------------------------------------*/

class Game {
public:
	Game(game_params params) : m_gen_num(params.n_gen), m_thread_num(params.n_thread),
	interactive_on(params.interactive_on), print_on(params.print_on), filename(params.filename),
	threads_in(0), cond_var(PTHREAD_COND_INITIALIZER), count_cond_var(PTHREAD_COND_INITIALIZER),
	threads_finished(0),game_end(0){
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
		pthread_mutex_init(&locker, &attr);
		pthread_mutex_init(&time_locker, &attr);
		pthread_mutex_init(&count_locker, &attr);
	}
	~Game();
	void run(); // Runs the game
	const vector<double> gen_hist() const; // Returns the generation timing histogram
	const vector<double> tile_hist() const; // Returns the tile timing histogram
	uint thread_num() const; //Returns the effective number of running threads = min(thread_num, field_height)
    void swap_fields();
	class Consumer : public Thread{
        public:
            Consumer(uint num, Game *game_ptr) : Thread(num), game(game_ptr) {}
            Game *game;
		int dom_color(vector<int> color_arr){
    int dom_species = 0;
    int dom_score = 0;
    int curr_score = 0;
    for(int i = 1; i < 8; i++){
        curr_score = (i)*(color_arr[i]);

        if(curr_score > dom_score){
            dom_species = i;
            dom_score = curr_score;
        }
    }
    return dom_species;
}
        protected:
		void thread_workload(){
            while(1){
                pthread_mutex_lock(&(game->locker));
                while(game->threads_in == game->m_thread_num){
                        pthread_cond_wait(&(game->cond_var),&(game->locker));
                }
                if(game->game_end == 1){
                    pthread_mutex_unlock(&(game->locker));
                    return;
                }

                game->threads_in++;
                pthread_mutex_unlock(&(game->locker));
                job_tile curr_job = game->jobs.pop();
                auto tile_start = std::chrono::system_clock::now();
                int height = curr_job.curr->size();
                int width = curr_job.curr->operator [](0).size();
                int i, j, k, l, ind_y, ind_x, cnt = 0, allive = 0,curr_color=0;
                //vector<int> dominant(8, 0);
                int currPhase = curr_job.current_phase;

                if (currPhase == 1){ //phase 1
                    for(i = curr_job.first_row; i<=curr_job.last_row; i++){
                        for(j=0; j<width; j++){
                            vector<int> dominant(8, 0);
                            cnt = 0;
                            allive = 0;
                            if((curr_job.curr->operator [](i).operator [](j)) != 0) {
                                curr_color = curr_job.curr->operator [](i).operator [](j);
                                allive = 1;
                                }
                            for(k=-1; k<=1; k++){
                                for(l=-1; l<=1; l++){
                                    if(l == 0 && k == 0) continue;
                                    ind_y = i + k;
                                    ind_x = j + l;
                                    if(ind_y < 0 || ind_y > height -1 || ind_x < 0 || ind_x > width -1) continue;
                                    if((curr_job.curr->operator [](ind_y).operator [](ind_x)) != 0){
                                        int tile_color = curr_job.curr->operator [](ind_y).operator [](ind_x);
                                        dominant[tile_color] = dominant[tile_color] + 1;
                                        cnt++;
                                    }

                                }
                            }

                            if (!allive && cnt == 3){
                                curr_job.next->operator [](i).operator [](j) = dom_color(dominant);
                            }

                            else if ((allive)&&(cnt == 2 || cnt == 3)){
                                curr_job.next->operator [](i).operator [](j) = curr_color;
                            }
                            else{
                                curr_job.next->operator [](i).operator [](j) = 0;
                            }
                        }
                    }
				}

                else{
                    for(i = curr_job.first_row; i<=curr_job.last_row; i++){
                        for(j=0; j<width; j++){
                            cnt = 0;
                            int upsum= 0;
                            if((curr_job.curr->operator [](i).operator [](j)) == 0) {
                                curr_job.next->operator [](i).operator [](j) = 0;
                                continue;
                            }
                            for(k=-1; k<=1; k++){
                                for(l=-1; l<=1; l++){
                                    ind_y = i + k;
                                    ind_x = j + l;
                                    if(ind_y < 0 || ind_y > height -1 || ind_x < 0 || ind_x > width -1) continue;
                                    int tile_color = curr_job.curr->operator [](ind_y).operator [](ind_x);
                                    if(tile_color != 0){
                                        upsum += tile_color;
                                        cnt++;
                                    }

                                }
                            }
                            float a = (float)upsum;
                            float b = (float)cnt;
                            int new_species = (int)round(a/b);
                            //printf("%f - " , a/b);
                            curr_job.next->operator [](i).operator [](j) = new_species;
                        }
                       // printf(" p %d \n " ,i);
                    }
                }

                auto tile_end = std::chrono::system_clock::now();
				pthread_mutex_lock(&(game->time_locker));
				game->m_tile_hist.push_back(
						(float) std::chrono::duration_cast
								< std::chrono::microseconds
								> (tile_end - tile_start).count());
				game->threads_finished++;
				if(game->threads_finished == game->m_thread_num){
					pthread_cond_signal(&(game->count_cond_var));
				}
				pthread_mutex_unlock(&(game->time_locker));
            }
		}
	}; //consumer



protected: // All members here are protected, instead of private for testing purposes

	// See Game.cpp for details on these three functions
	void _init_game();
	void _step(uint curr_gen);
	void _destroy_game();
	inline void print_board(const char* header);

	uint m_gen_num; 			 // The number of generations to run
	uint m_thread_num; 			 // Effective number of threads = min(thread_num, field_height)
	vector<double> m_tile_hist; 	 // Shared Timing history for tiles: First (2 * m_gen_num) cells are the calculation durations for tiles in generation 1 and so on.
							   	 // Note: In your implementation, all m_thread_num threads must write to this structure.
	vector<double> m_gen_hist;  	 // Timing history for generations: x=m_gen_hist[t] iff generation t was calculated in x microseconds
	vector<Thread*> m_threadpool; // A storage container for your threads. This acts as the threadpool.

	bool interactive_on; // Controls interactive mode - that means, prints the board as an animation instead of a simple dump to STDOUT
	bool print_on; // Allows the printing of the board. Turn this off when you are checking performance (Dry 3, last question)

	// TODO: Add in your variables and synchronization primitives
	vector<vector<int>> curr;
	vector<vector<int>> next;
	string filename;
	PCQueue<struct job_tile> jobs;
	uint threads_in;
	pthread_mutex_t locker;
	pthread_cond_t cond_var;
	pthread_mutex_t time_locker;
	pthread_mutex_t count_locker;
	pthread_cond_t count_cond_var;
	uint threads_finished;
	pthread_mutexattr_t attr;
	int game_end;

};



#endif
