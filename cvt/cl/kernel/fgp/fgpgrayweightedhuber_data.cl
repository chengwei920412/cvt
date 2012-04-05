__kernel void fgp_data( __write_only image2d_t output, __read_only image2d_t ein, __read_only image2d_t img, const float lambda, const float t, __local float4* buf  )
{
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	const sampler_t samplerb = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	const int width = get_image_width( img );
	const int height = get_image_height( img );
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
	const int lw = get_local_size( 0 );
	const int lh = get_local_size( 1 );
	const int bstride = lw + 1;
	const int2 base2 = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
	int2 coord;
	float2 delta;

	for( int y = ly; y < lh + 1; y += lh ) {
		for( int x = lx; x < lw + 1; x += lw ) {
			// get vec e_t, e_{t-1}
			float4 tmp = read_imagef( ein, samplerb, base2 + ( int2 ) ( x, y ) );
			// store p_t and e_t
			buf[ mul24( y, bstride ) + x ] = ( float4 ) ( tmp.xy * ( 1.0f + t ) - t * tmp.zw, tmp.xy );
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) ( buf[ mul24( ( y ) + 1, bstride ) + ( ( x ) + 1 ) ] )

	if( gx >= width || gy >= height )
		return;

	float2 center = BUF( lx, ly ).xy;
	float  top    = BUF( lx, ly - 1 ).y;
	float  left   = BUF( lx - 1, ly ).x;
	delta = center - ( float2 ) ( left, top );
	// image + lambda * div( p )
	float4 pixel = read_imagef( img, sampler, ( int2 )( gx, gy ) );
	pixel.x += lambda * pixel.y * ( delta.x + delta.y );

	write_imagef( output, ( int2 ) ( gx, gy ), pixel );
}
