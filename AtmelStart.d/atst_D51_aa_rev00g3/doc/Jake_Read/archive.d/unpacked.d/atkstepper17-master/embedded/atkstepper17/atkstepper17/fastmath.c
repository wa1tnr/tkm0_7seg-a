/*
 * fastmath.c
 *
 * Created: 6/20/2018 12:38:38 PM
 *  Author: Jake
 */ 

int fastModulo(int dividend, int divisor){
	// divisor must be a power of 2!
	return dividend & (divisor - 1);
}