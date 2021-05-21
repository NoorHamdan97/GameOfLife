#include "Game.hpp"


static const char *colors[7] = {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN};
/*--------------------------------------------------------------------------------

--------------------------------------------------------------------------------*/
Game::~Game(){

}

void Game::run() {
	_init_game(); // Starts the threads and all other variables you need
	print_board("Initial Board");
	for (uint i = 0; i < m_gen_num; ++i) {
		auto gen_start = std::chrono::system_clock::now();
		_step(i); // Iterates a single generation
		auto gen_end = std::chrono::system_clock::now();
		m_gen_hist.push_back((double)std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
		print_board(nullptr);
	} // generation loop
	print_board("Final Board");
	_destroy_game();
}

int stringToint(string k) {

	if(k.compare("1")==0){return 1;}
	if(k.compare("2")==0){return 2;}
	if(k.compare("3")==0){return 3;}
	if(k.compare("4")==0){return 4;}
	if(k.compare("5")==0){return 5;}
	if(k.compare("6")==0){return 6;}
	if(k.compare("7")==0){return 7;}
	return 0;
}

void Game::_init_game() {
	// Create game fields - Consider using utils:read_file, utils::split
	//need to do it with colors
	//curr = new vector<vector<int>>();
	//next = new vector<vector<int>>();
	vector<string> string_curr = utils::read_lines(filename);
	vector<vector<string>> vec_str_curr;
	for(uint i=0; i<string_curr.size(); i++){
		vec_str_curr.push_back(utils::split(string_curr.operator [](i),' '));
	}

	for(uint i=0; i<vec_str_curr.size(); i++){
		vector<int> curr_vec;
		for(uint j=0; j<vec_str_curr.operator [](0).size(); j++){
			int toAdd = stringToint(vec_str_curr.operator [](i).operator [](j));
            curr_vec.push_back(toAdd);
		}
		curr.push_back(curr_vec);
	}

	next = curr;

	if(curr.size() < m_thread_num) m_thread_num = curr.size();
	// Create & Start threads
	// Create threads
	for(uint i=0; i<m_thread_num; i++) {
		Consumer *new_consumer = new Consumer(i,this);
		m_threadpool.push_back(new_consumer);
	}
	// Start the threads
	for(auto it = m_threadpool.begin(); it != m_threadpool.end(); it++){
		((Consumer*)*it)->start();
	}
	// Testing of your implementation will presume all threads are started here
}


void Game::swap_fields() {
    vector<vector<int>> temp;
    temp = curr;
    curr = next;
    next = temp;
    //curr.swap(next);
}

void Game::_step(uint curr_gen) {
		int tile = curr.size()/m_thread_num;
	struct job_tile curr_job;
	for(uint i=0; i<m_thread_num; i++){
		//struct job_tile curr_job;
		curr_job.first_row = i*tile;
		curr_job.last_row = (i+1)*tile - 1;
		if(i == m_thread_num - 1){
			curr_job.last_row = curr.size() - 1;
		}
		curr_job.curr = &curr;
		curr_job.next = &next;
		curr_job.current_phase=1;
		jobs.push(curr_job);
	}
	pthread_mutex_lock(&time_locker);
	while(threads_finished != m_thread_num){
		pthread_cond_wait(&count_cond_var,&time_locker);
	}
	threads_finished = 0;
	pthread_mutex_unlock(&time_locker);

	pthread_mutex_lock(&locker);
	//if(curr_gen == m_gen_num - 1){game_end = 1;}
	swap_fields();
	threads_in = 0;
	pthread_cond_broadcast(&cond_var);
	pthread_mutex_unlock(&locker);

	for(uint i=0; i<m_thread_num; i++){
		curr_job.first_row = i*tile;
		curr_job.last_row = (i+1)*tile - 1;
		if(i == m_thread_num - 1){
			curr_job.last_row = curr.size() - 1;
		}
		curr_job.curr = &curr;
		curr_job.next = &next;
		curr_job.current_phase=2;
		jobs.push(curr_job);

	}
	pthread_mutex_lock(&time_locker);
	while(threads_finished != m_thread_num){
		pthread_cond_wait(&count_cond_var,&time_locker);
	}
	threads_finished = 0;
	pthread_mutex_unlock(&time_locker);
	pthread_mutex_lock(&locker);

	if(curr_gen == m_gen_num - 1){game_end = 1;}

	swap_fields();
	threads_in = 0;
	pthread_cond_broadcast(&cond_var);
	pthread_mutex_unlock(&locker);


}

void Game::_destroy_game(){

	// Destroys board and frees all threads and resources
	// Not implemented in the Game's destructor for testing purposes.
	// All threads must be joined here
	for(auto it = m_threadpool.begin(); it != m_threadpool.end(); it++){
		((Consumer*)*it)->join();
	}
}


const vector<double> Game::gen_hist() const{
	return m_gen_hist;
}

const vector<double> Game::tile_hist() const{
	return m_tile_hist;
}

uint Game::thread_num() const{
	return m_thread_num;
}
/*--------------------------------------------------------------------------------

--------------------------------------------------------------------------------*/
inline void Game::print_board(const char* header) {

	if(print_on){

		// Clear the screen, to create a running animation
		if(interactive_on)
			system("clear");

		// Print small header if needed
		if (header != nullptr)
			cout << "<------------" << header << "------------>" << endl;

		// TODO: Print the board

		cout << u8"╔" << string(u8"═") * curr.operator [](0).size() << u8"╗" << endl;
		for (uint i = 0; i < curr.size(); ++i) {
			cout << u8"║";
			for (uint j = 0; j < curr.operator [](0).size(); ++j) {
				if (curr[i][j] > 0)
                    cout << colors[curr[i][j] % 7] << u8"█" << RESET;
                else
                    cout << u8"░";
			}
			cout << u8"║" << endl;
		}
		cout << u8"╚" << string(u8"═") * curr.operator [](0).size() << u8"╝" << endl;

		// Display for GEN_SLEEP_USEC micro-seconds on screen
		if(interactive_on)
			usleep(GEN_SLEEP_USEC);
	}

}


/* Function sketch to use for printing the board. You will need to decide its placement and how exactly
	to bring in the field's parameters.

		cout << u8"╔" << string(u8"═") * field_width << u8"╗" << endl;
		for (uint i = 0; i < field_height ++i) {
			cout << u8"║";
			for (uint j = 0; j < field_width; ++j) {
                if (field[i][j] > 0)
                    cout << colors[field[i][j] % 7] << u8"█" << RESET;
                else
                    cout << u8"░";
			}
			cout << u8"║" << endl;
		}
		cout << u8"╚" << string(u8"═") * field_width << u8"╝" << endl;
*/



