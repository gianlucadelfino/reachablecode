#include <msws.cl>

ulong _msws_ulong2(msws_state* state){
	state->x *= state->x;
	state->x += (state->w += 0xb5ad4eceda1ce2a9);
	return state->x = (state->x>>32) | (state->x<<32);
}
kernel void calculate_trajectory(
    float share_value_,
    const float rate_,
    const float dt_,
    int num_iterations_,
    const float variance_,
    global float* final_values_)
{
    // Get the index of the current element to be processed
    const ulong i = get_global_id(0);

    // This random algorithm multiplies the seed by a big number, so we need
    // big seeds to see a difference;
    const ulong msws_const = 0xb5ad4eceda1ce2a9;
    const ulong seed = (i + 1) * msws_const;

    // Use global id to seed random algorithm
    msws_state state;
    msws_seed(&state, seed);

    const float two_pi = 2.f * M_PI;

    while(num_iterations_--)
    {
        // Compute gauss random number using box muller
        const float rand1 = msws_float(state);
        const float rand2 = msws_float(state);

        const float rand1_log = -2 * log(rand1);

        float cos_rand2;
        const float sin_rand2 = sincos(rand2, &cos_rand2);

        const float gauss_rand1 = variance_ * sqrt(rand1_log) * cos_rand2;
        const float gauss_rand2 = variance_ * sqrt(rand1_log) * sin_rand2;

        // Now increment the value "dt*r + epsilon*dW"
        share_value_ += dt_ * rate_ + gauss_rand1 * sqrt(dt_);
        share_value_ += dt_ * rate_ + gauss_rand2 * sqrt(dt_);
    }

    final_values_[i] = share_value_;
}