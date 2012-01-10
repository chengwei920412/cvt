__kernel void fgp_data( __write_only image2d_t out, __read_only image2d_t img, __read_only image2d_t e1, __read_only image2d_t e0, const float lambda, const float t, __local float4* buf  )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
    const sampler_t samplerb = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int bstride = ( lw << 1 ) + 4;
	const int2 base2 = ( int2 )( get_group_id( 0 ) * ( lw << 1 ) - 2, get_group_id( 1 ) * lh - 1 );
	int2 coord;
	float4 val, dx, dy;

	for( int y = ly; y < lh + 1; y += lh ) {
		coord.y = y;
		for( int x = lx; x < lw + 1; x += lw ) {
			coord.x = x << 1;
			// px
			buf[ mul24( y, bstride ) + coord.x ] = read_imagef( e1, samplerb, base2 + coord ) * ( 1.0f + t ) - t * read_imagef( e0, samplerb, base2 + coord );
			// py
			coord.x += 1;
			buf[ mul24( y, bstride ) + coord.x ] = read_imagef( e1, samplerb, base2 + coord ) * ( 1.0f + t ) - t * read_imagef( e0, samplerb, base2 + coord );
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

	if( gx >= get_image_width( img ) || gy >= get_image_height( img ) )
		return;

#define XBUF( x, y ) ( buf[ mul24( ( ( y ) + 1 ), bstride ) + ( ( ( ( x ) + 1 ) << 1 ) ) ] )
#define YBUF( x, y ) ( buf[ mul24( ( ( y ) + 1 ), bstride ) + ( ( ( ( x ) + 1 ) << 1 ) + 1 ) ] )

	dx = XBUF( lx, ly ) - XBUF( lx - 1, ly );
	dy = YBUF( lx, ly ) - YBUF( lx, ly - 1 );
	val = read_imagef( img, sampler, ( int2 )( gx, gy ) ) + lambda * ( dx + dy );

	write_imagef( out, ( int2 ) ( gx, gy ), val );
}