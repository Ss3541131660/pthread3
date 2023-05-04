#include <pthread.h>
#include <iostream>
#include<time.h>
#include <semaphore.h>
#define NUM_THREADS 4
using namespace std;
int n = 128;
float pA[128][128];
pthread_mutex_t mutex;

typedef struct {
	int k;          // ��ȥ���ִ�
	int t_id;       // �߳�id
} threadParam_t;
//�ṹ�嶨��

sem_t sem_leader;
sem_t sem_Divsion[NUM_THREADS - 1];
sem_t sem_Elimination[NUM_THREADS - 1];

void* threadFunc(void* param) {
	threadParam_t* p = (threadParam_t*)param;
	int t_id = p->t_id;

	for (int k = 0; k < n; ++k) {
		if (t_id == 0) {//ѡ��һ��t_idΪ0���߳����������������������̵߳ȴ�
			for (int j = k + 1; j < n; j++) {
				pA[k][j] = pA[k][j] / pA[k][k];
			}
			pA[k][k] = 1.0;
		}
		else {
			sem_wait(&sem_Divsion[t_id - 1]); //�������ȴ�������ɳ��������������Լ�ר�����ź�����
		}
		//������ɺ������������̣߳�������ȥ����
		if (t_id == 0) {
			for (int i = 0; i < NUM_THREADS - 1; ++i) {
				sem_post(&sem_Divsion[i]);
			}
		}

		// ѭ����������
		for (int i = k + 1 + t_id; i < n; i += NUM_THREADS) {
			// ��ȥ
			for (int j = k + 1; j < n; ++j) {
				pA[i][j] = pA[i][j] - pA[i][k] * pA[k][j];
			}
			pA[i][k] = 0.0;
		}

		if (t_id == 0) {
			for (int i = 0; i < NUM_THREADS - 1; ++i) {
				sem_wait(&sem_leader);//�ȴ������Ĺ����߳������ȥ����
			}
			for (int i = 0; i < NUM_THREADS - 1; ++i) {
				sem_post(&sem_Elimination[i]);//֪ͨ���������߳̽�����һ��
			}
		}
		else {
			sem_post(&sem_leader); //ͬ־leadder���Ѿ������ȥ����
			sem_wait(&sem_Elimination[t_id - 1]); //�ȴ�֪ͨ��������һ��
		}
	}
	pthread_exit(NULL);
}

int main() {
	// ����A��n
	// ...
	srand(time(NULL));
	for (int i = 0; i < n; i++) {
		pA[i][i] = 1.0;//�Խ���Ϊ1.0
		for (int j = 0; j < n; j++) {
			if (j >i)pA[i][j] = rand() % 10;
			else if(j<i)pA[i][j] = 0;
		}
	}
	//�����Ǿ���
	for (int k = 0; k < n; k++) {
		for (int i = k + 1; i < n; i++) {
			for (int j = 0; j < n; j++) {
				pA[i][j] += pA[k][j];
			}
		}
	}

	sem_init(&sem_leader, 0, 0);
	for (int i = 0; i < NUM_THREADS; ++i) {
		sem_init(&sem_Divsion[i], 0, 0);
		sem_init(&sem_Elimination[i], 0, 0);
	}

	// �����߳�
	pthread_t handles[NUM_THREADS];
	threadParam_t param[NUM_THREADS];
	for (int t_id = 0; t_id < NUM_THREADS; t_id++) {
		param[t_id].t_id = t_id;
		pthread_create(&handles[t_id], NULL, threadFunc, &param[t_id]);
	}

	for (int t_id = 0; t_id < NUM_THREADS; t_id++) {
		pthread_join(handles[t_id], NULL);
	}

	// ���������ź���
	sem_destroy(&sem_leader);
	for (int i = 0; i < NUM_THREADS; ++i) {
		sem_destroy(&sem_Divsion[i]);
		sem_destroy(&sem_Elimination[i]);
	}

	return 0;
}
