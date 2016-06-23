#include "FNN.h"
#include "record.h"

/* model of the wheelchair */
float Ap[2][2] = {{-0.105f, 0.0532f}, {0.047f, -0.1027}};
float Bp[2][2] = {{0.3701f, -0.1712f}, {-0.1481f, 0.3170f}};
float Bi[2][2] = {{4.403f, 2.378f}, {2.057f, 4.265f}};
/* Bi is the inverse matrix of Bp */

float st = 0.02f; /* sampling time(s) */ 

/* init Weight (random) */
float w1[3][3] = {{0.0f, 0.0f, 0.0f} \
				 ,{0.0f, 0.0f, 0.0f} \
				 ,{0.0f, 0.0f, 0.0f}};

float w2[3][3] = {{0.0f, 0.0f, 0.0f} \
				 ,{0.0f, 0.0f, 0.0f} \
				 ,{0.0f, 0.0f, 0.0f}};


/* init switching ctrl value (random)*/
float P[2] = {0.0f, 0.0f};

/* output sets 
 * yt = yfnc + yr + ys
 * 0: left , 1: right */
float k = 5.1f;/* gain of yt for real ctrl signal */
float yt[2];
float yfnc[2];
float yr[2];
float ys[2];
int fnn_l = 0, fnn_r = 0;


float xl, xl_1;
float xr, xr_1;

/* membership function */
MBF_t mbl1, mbl2, mbl3;
MBF_t mbr1, mbr2, mbr3;

void initMBF(MBF_t *mf, float b, float c){
	mf->c = c;
	mf->b = b;
}

void initFNN(){
	/* left */
	initMBF(&mbl1, 3.0f, 5.172f);
	initMBF(&mbl2, 3.0f, 0.0f);
	initMBF(&mbl3, 3.0f, -5.172f);
	/* right */
	initMBF(&mbr1, 3.0f, 5.172f);
	initMBF(&mbr2, 3.0f, 0.0f);
	initMBF(&mbr3, 3.0f, -5.172f);

	xl = 0.0f;
	xr = 0.0f;
	xl_1 = 0.0f;
	xr_1 = 0.0f;

/*	int i,j;
	for(i=0; i<num; i++){
		for(j=0; j<num; j++){
			w1[i][j] = 0.0f;
			w2[i][j] = 0.0f;			
		}
	}
*/
	P[0] = 0.0f;
	P[1] = 0.0f;
}

void fzyNeuCtrl(int en_l, int en_r, float set_l, float set_r){
	/* encoder value ----> rotation (rad/s) */
	float vl, vr;
	vl = (float)en_l * 0.2586f;
	vr = (float)en_r * 0.2586f;
	set_l = set_l * 0.2586f;
	set_r = set_r * 0.2586f;

	/* update x(m) with x(m+1) computed last time */
	xl = xl_1; 
	xr = xr_1;

	/* 1.reference model */
	xl_1 = referModel(xl, set_l);
	xr_1 = referModel(xr, set_r);
	
	/* 2.reference state */
	referState(xl_1, xr_1, xl, xr);

	/* 3.error */
	float e1, e2;
	e1 = (xl - vl);
	e2 = (xr - vr);

	/* 4.fuzzy neural controller */
	fzyNeu_update(e1, e2);

	/* 5.switching control */
	switching(e1, e2);

	/* 6.control signal */
	yt[0] = yfnc[0] + ys[0] + yr[0];
	yt[1] = yfnc[1] + ys[1] + yr[1];

	fnn_l = math_round(k*yt[0]);
	fnn_r = math_round(k*yt[1]);

	recFzyNeuData((int)(e1*10000), (int)(e2*10000), (int)(yfnc[0]*10000), (int)(yfnc[1]*10000), (int)(ys[0]*10000), (int)(ys[1]*10000), (int)(yr[0]*10000), (int)(yr[1]*10000));
}

float compute_MBF(MBF_t *mf, float err){
	float tmp = math_pow((err - mf->c), 2);
	tmp = tmp/ math_pow(mf->b, 2);
	tmp = exponential(tmp);
	return tmp;
}

/* reference model */
/* X(m+1) = a*X(m) + b*r(m) */
float referModel(float xm, float rm){
	float xm_1, a, b;
	a = 0.5f;
	b = 0.5f;
	xm_1 = a * xm + b * rm;

	return xm_1;
}

/* reference state */
void referState(float v11, float v21, float v1, float v2){
/* vn1 = Xn(m+1), vn = Xn(m), n=1 for left, n=2 for right */

	float x[2];
	x[0] = v11 - (Ap[0][0]*v1 + Ap[0][1]*v2);
	x[1] = v21 - (Ap[1][0]*v1 + Ap[1][1]*v2);

	yr[0] = Bi[0][0]*x[0] + Bi[0][1]*x[1];
	yr[1] = Bi[1][0]*x[0] + Bi[1][1]*x[1];
}

void switching(float e1, float e2){
	/* dP = |e * Bp| */
	/* P = P + dP*t */
	/* PP = sign(dP)*P */
	float dp[2];
	dp[0] =  e1 * Bp[0][0] + e2 * Bp[1][0];
	dp[1] =  e1 * Bp[0][1] + e2 * Bp[1][1];

	P[0] = P[0] + f_abs(dp[0]) * st;
	P[1] = P[1] + f_abs(dp[1]) * st;

	ys[0] = f_sign(dp[0])*P[0];
	ys[1] = f_sign(dp[1])*P[1];
}

void fzyNeu_update(float e1, float e2){
	float a1[num];
	a1[0] = compute_MBF(&mbl1, e1);
	a1[1] = compute_MBF(&mbl2, e1);
	a1[2] = compute_MBF(&mbl3, e1);

	float a2[num];
	a2[0] = compute_MBF(&mbr1, e2);
	a2[1] = compute_MBF(&mbr2, e2);
	a2[2] = compute_MBF(&mbr3, e2);

	float d[num][num];
	float sum;
	int i = 0, j = 0;
	int cnt;
	for(i=0; i<num; i++){
		for(j=0; j<num; j++){
			d[i][j] = a1[i] * a2[j];
			sum += d[i][j];
		}
	}

	cnt = 0;
	float phi[num*num];
	for(i=0; i<num; i++){
		for(j=0; j<num; j++){
			phi[cnt++] = d[i][j] / sum;
		}
	}

	/* Weight Update */
	/* d_Weight = (phi' * e * Bp) */
	float dw1[rule_num];
	float dw2[rule_num];
	float fw1[rule_num];
	float fw2[rule_num];

	for(cnt = 0; cnt < num*num ; cnt++){
		dw1[cnt] = phi[cnt] * e1;
		dw2[cnt] = phi[cnt] * e2;

		fw1[cnt] = dw1[cnt] * Bp[0][0] + dw2[cnt] * Bp[1][0];
		fw2[cnt] = dw1[cnt] * Bp[0][1] + dw2[cnt] * Bp[1][1];
	}

	cnt = 0;
	for(i=0; i<num; i++){
		for(j=0; j<num; j++){
			w1[i][j] = w1[i][j] + st * fw1[cnt];
			w2[i][j] = w2[i][j] + st * fw2[cnt];
			cnt++;
		}
	}

	cnt = 0;
	float y1 = 0, y2 = 0;
	for(i=0; i<num; i++){
		for(j=0; j<num; j++){
			y1 += phi[cnt] * w1[i][j];
			y2 += phi[cnt] * w2[i][j];
			cnt++;
		}
	}

	yfnc[0] = y1;
	yfnc[1] = y2;
}