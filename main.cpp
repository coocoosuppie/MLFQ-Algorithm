// Jason Schilling
// Z23270204
// MLFQ Algorithm
// quick and dirty!

#include <forward_list>
#include <iostream>
enum state { waiting, executing, inIO, completed };

struct process {
	process(int array1[], int array2[], int cpuTimeSize, int ioTimeSize, char name) : cpuTimeSize(cpuTimeSize), ioTimeSize(ioTimeSize), name(name) {
		cpuTime = array1;
		ioTime = array2;
	};

	char name; // process name

	// initialize variables
	int * cpuTime, *ioTime, cpuTimeSize, ioTimeSize, currentBurst = 0, ioTimer = 0, currentIO = 0, cpuTimer = 0;
	state procState = waiting;
	unsigned int arrivalTime = 0;
	bool responseFlag = false, doneForCycle = false;
	int queueLevel = 1; // starting queue level
	int responseTime = 0, waitTime = 0, turnaroundTime = 0; // for calculations
};

// FUNCTION DECLARATIONS
void mlfq(std::forward_list<process>);
void printCalculatedData(std::forward_list<process> Processes, unsigned int idleTime);
void incrementIO(std::forward_list<process> & processes, unsigned int time);
int getMinLevel(std::forward_list<process> processes);

int main(void) {
	
	// HARD CODE ALL DATA ///////////////////////////////
	int temp1CPU[]{ 5, 3, 5, 4, 6, 4, 3, 4 };
	int temp1IO[]{ 27, 31, 43, 18, 22, 26, 24 };
	process P1(temp1CPU, temp1IO, 8, 7, '1');

	int temp2CPU[]{ 4, 5, 7, 12, 9, 4, 9, 7, 8 };
	int temp2IO[]{ 48, 44, 42, 37, 76, 41, 31, 43 };
	process P2(temp2CPU, temp2IO, 9, 8, '2');

	int temp3CPU[]{ 8, 12, 18, 14, 4, 15, 14, 5, 6 };
	int temp3IO[]{ 33, 41, 65, 21, 61, 18, 26, 31 };
	process P3(temp3CPU, temp3IO, 9, 8, '3');

	int temp4CPU[]{ 3, 4, 5, 3, 4, 5, 6, 5, 3 };
	int temp4IO[]{ 35, 41, 45, 51, 61, 54, 82, 77 };
	process P4(temp4CPU, temp4IO, 9, 8, '4');

	int temp5CPU[]{ 16, 17, 5, 16, 7, 13, 11, 6, 3, 4 };
	int temp5IO[]{ 24, 21, 36, 26, 31, 28, 21, 13, 11 };
	process P5(temp5CPU, temp5IO, 10, 9, '5');

	int temp6CPU[]{ 11, 4, 5, 6, 7, 9, 12, 15, 8 };
	int temp6IO[]{ 22, 8, 10, 12, 14, 18, 24, 30 };
	process P6(temp6CPU, temp6IO, 9, 8, '6');

	int temp7CPU[]{ 14, 17, 11, 15, 4, 7, 16, 10 };
	int temp7IO[]{ 46, 41, 42, 21, 32, 19, 33 };
	process P7(temp7CPU, temp7IO, 8, 7, '7');

	int temp8CPU[]{ 4, 5, 6, 14, 16, 6 };
	int temp8IO[]{ 14, 33, 51, 73, 87 };
	process P8(temp8CPU, temp8IO, 6, 5, '8');
	// END HARD CODING /////////////////////////////////////////

	std::forward_list<process> processes = {P1, P2, P3, P4, P5, P6, P7, P8};

	mlfq(processes);

	system("pause");
	return 0;
}

void mlfq(std::forward_list<process> processes) {
	int time = 0;
	unsigned int minArrivalTime = -1, minLevel = 3, idleTime = 0;
	process * processToWork = nullptr, * prevProcess = nullptr;
	bool working = false, idleFlag = false, notComplete = true;
	while (time < 590 && notComplete) {
		minLevel = getMinLevel(processes);
		processToWork = nullptr;
		minArrivalTime = -1;
		notComplete = false;
		for (auto x = processes.begin(); x != processes.end(); x++) {
			// find the highest priority process that came in at the earliest time
			if (x->arrivalTime < minArrivalTime && (x->procState == waiting || x->procState == executing) && x->queueLevel == minLevel) {
				minArrivalTime = x->arrivalTime;
				processToWork = &*x;
			}
		}

		// if process get interrupted, handle house-keeping for it
		if (processToWork != prevProcess && prevProcess != nullptr) {
			if (prevProcess->procState == executing) {
				prevProcess->cpuTime[prevProcess->currentBurst] -= prevProcess->cpuTimer;
				if (prevProcess->cpuTime[prevProcess->currentBurst] == 0) {
					// burst is done, move on
					prevProcess->arrivalTime = time;
					if (prevProcess->currentBurst < prevProcess->cpuTimeSize - 1) {
						prevProcess->currentBurst++;
						prevProcess->cpuTimer = 0;
						working = false;
						prevProcess->procState = inIO;
					}
					// nothing more to do for this process
					else {
						prevProcess->cpuTimer = 0;
						prevProcess->procState = completed;
						working = false;
					}
				}
				else {
					prevProcess->arrivalTime = time;
					prevProcess->cpuTimer = 0;
					prevProcess->procState = waiting;
					working = false;
				}
			}
		}

		// calculations and checks loop
		for (auto x = processes.begin(); x != processes.end(); x++) {
			// calculate wait time
			if (&*x != processToWork && x->procState == waiting) {
				x->waitTime++;
			}
			// check if everything is done
			if (x->procState != completed) {
				notComplete = true;
			}
		}

		// nothing to do
		if (processToWork == nullptr) {
			incrementIO(processes, time);
			idleTime++;
			// print info ///////////////////////////////////////////////////////////////////////
			if (!idleFlag) {
				std::cout << "-----------------------------------\n";
				std::cout << "Current Time: " << time << "\n";
				std::cout << "Next Process on the CPU: idle\n";

				std::cout << "List of processes in the ready queue:" << "\n";
				std::cout << "Process\tBurst\n";
				for (auto x = processes.begin(); x != processes.end(); x++) {
					if (&*x != processToWork && x->procState == waiting) {
						std::cout << "P" << x->name << "\t" << x->cpuTime[x->currentBurst] << "\n";
					}
				}

				std::cout << "List of processes in I/O:" << "\n";
				std::cout << "Process\tRemaining I/O time\n";
				for (auto x = processes.begin(); x != processes.end(); x++) {
					if (&*x != processToWork && x->procState == inIO) {
						std::cout << "P" << x->name << "\t" << x->ioTime[x->currentIO] << "\n";
					}
				}
				std::cout << "-----------------------------------\n\n";
				//////////////////////////////////////////////////////////////////////////////////

				idleFlag = true;
			}
			time++;
		}

		// something to do!
		else {
			switch (processToWork->procState) {
			case waiting:
				if (!working) {
					std::cout << "-----------------------------------\n";
					std::cout << "Current Time: " << time << "\n";
					std::cout << "Next Process on the CPU: P" << processToWork->name << "\n";

					std::cout << "List of processes in the ready queue:" << "\n\n";
					std::cout << "Process\tBurst\tQueue\n";
					for (auto x = processes.begin(); x != processes.end(); x++) {
						if (&*x != processToWork && x->procState == waiting) {
							std::cout << "P" << x->name << "\t" << x->cpuTime[x->currentBurst] << "\t" << x->queueLevel << "\t" << "\n";
						}
					}

					std::cout << "List of processes in I/O:" << "\n\n";
					std::cout << "Process\tRemaining I/O time\n";
					for (auto x = processes.begin(); x != processes.end(); x++) {
						if (&*x != processToWork && x->procState == inIO) {
							std::cout << "P" << x->name << "\t" << x->ioTime[x->currentIO] << "\n";
						}
					}
					std::cout << "-----------------------------------\n\n";

					idleFlag = false;

					processToWork->cpuTimer = 0;
					processToWork->ioTimer = 0;
					processToWork->procState = executing;
					working = true;
				}
				break;
			case executing:

				// calculate response time if it hasn't been done
				if (!processToWork->responseFlag) {
					processToWork->responseTime = time;
					processToWork->responseFlag = true;
				}
				if (processToWork->cpuTimer < processToWork->cpuTime[processToWork->currentBurst]) {

					// do different things depending on queue level
					switch (processToWork->queueLevel) {
					case 1:
						if (processToWork->cpuTimer < 5) {
							processToWork->cpuTimer++;
							incrementIO(processes, time);
							time++;
						}
						// downgrade priority
						else {
							processToWork->queueLevel++;
							processToWork->cpuTime[processToWork->currentBurst] -= 5;
							processToWork->cpuTimer = 0;
							processToWork->arrivalTime = time;
							working = false;
							processToWork->procState = waiting;
						}
						break;
					case 2:
						if (processToWork->cpuTimer < 10) {
							processToWork->cpuTimer++;
							incrementIO(processes, time);
							time++;
						}
						// downgrade priority
						else {
							processToWork->queueLevel++;
							processToWork->cpuTime[processToWork->currentBurst] -= 10;
							processToWork->cpuTimer = 0;
							processToWork->arrivalTime = time;
							working = false;
							processToWork->procState = waiting;
						}
						break;
					case 3:
						processToWork->cpuTimer++;
						incrementIO(processes, time);
						time++;	
						break;
					}
				}
				else {
					// burst is done, move on
					processToWork->arrivalTime = time;
					if (processToWork->currentBurst < processToWork->cpuTimeSize - 1) {
						processToWork->currentBurst++;
						processToWork->cpuTimer = 0;
						working = false;
						processToWork->procState = inIO;
					}
					// nothing more to do for this process
					else {
						processToWork->cpuTimer = 0;
						processToWork->procState = completed;
						working = false;
					}
				}
				break;
			case completed:
				std::cout << "completed\n";
				break;
			}
			minArrivalTime = -1;
		}
		prevProcess = processToWork;
	}
	printCalculatedData(processes, idleTime);
}

// find the highest priority level
int getMinLevel(std::forward_list<process> processes) {
	int minLevel = 3;
	for (auto x = processes.begin(); x != processes.end(); x++) {
		// find highest priority available given appropriate processes.
		if (x->queueLevel < minLevel && (x->procState == waiting || x->procState == executing)) {
			minLevel = x->queueLevel;
		}
	}
	return minLevel;
}

// print data
void printCalculatedData(std::forward_list<process> processes, unsigned int idleTime) {
	std::cout << "\nProcess\tRT\tWT\tTT\n";
	float avgRT = 0, avgWT = 0, avgTT = 0;
	for (auto x = processes.begin(); x != processes.end(); x++) {
		std::cout << "P" << x->name << "\t" << x->responseTime << "\t" << x->waitTime << "\t" << x->turnaroundTime << "\n";
		avgRT += x->responseTime;
		avgWT += x->waitTime;
		avgTT += x->turnaroundTime;
	}
	avgRT /= 8;
	avgWT /= 8;
	avgTT /= 8;
	std::cout << "AVERAGES:\nRT\tWT\tTT\n" << avgRT << "\t" << avgWT << "\t" << avgTT << "\n\n";
	std::cout << "CPU UTILIZATION: " << 100 - ((idleTime / 590.0) * 100) << "%\n";
}

// go through each process in IO and do one time unit of work
void incrementIO(std::forward_list<process> & processes, unsigned int time) {
	for (auto x = processes.begin(); x != processes.end(); x++) {
		if (x->procState == inIO) {
			x->ioTime[x->currentIO]--;
			if (x->ioTime[x->currentIO] <= 0) {
				if (x->currentIO < x->ioTimeSize - 1) {
					x->currentIO++;
					x->procState = waiting;
				}
				else {
					x->procState = waiting;
				}
				x->turnaroundTime = time;
				x->arrivalTime = time;
			}
		}
	}
}