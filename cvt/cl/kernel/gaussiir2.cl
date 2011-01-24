__kernel void gaussiir2( __write_only image2d_t output, __global float4* buffer, __global float4* buffer2, const int h, const float4 n, const float4 m, const float4 d )
{
	int2 coord;
    float4 x[ 4 ];
    float4 y[ 4 ];
	float4 xn, yn;
	const int stride = get_global_size( 0 );

    coord.x = get_global_id( 0 );

	buffer += get_global_id( 0 );
	buffer2 += get_global_id( 0 ) * h;

	float b1 = ( n.s0 + n.s1 + n.s2 + n.s3 ) / ( d.s0 + d.s1 + d.s2 + d.s3 + 1.0f );
	float b2 = ( m.s0 + m.s1 + m.s2 + m.s3 ) / ( d.s0 + d.s1 + d.s2 + d.s3 + 1.0f );

	// forward pass
	x[ 0 ] = buffer[ 0 ];
	x[ 1 ] = buffer[ stride ];
	x[ 2 ] = buffer[ 2 * stride ];
	x[ 3 ] = buffer[ 3 * stride ];

	y[ 0 ] = n.s0 * x[ 0 ] + n.s1 * x[ 0 ] + n.s2 * x[ 0 ] + n.s3 * x[ 0 ]
		- b1 * ( d.s0 * x[ 0 ] + d.s1 * x[ 0 ] + d.s2 * x[ 0 ] + d.s3 * x[ 0 ] );
	y[ 1 ] = n.s0 * x[ 1 ] + n.s1 * x[ 0 ] + n.s2 * x[ 0 ] + n.s3 * x[ 0 ]
		- d.s0 * y[ 0 ] - b1 * ( d.s1 * x[ 0 ] + d.s2 * x[ 0 ] + d.s3 * x[ 0 ] );
	y[ 2 ] = n.s0 * x[ 2 ] + n.s1 * x[ 1 ] + n.s2 * x[ 0 ] + n.s3 * x[ 0 ]
		- d.s0 * y[ 1 ] - d.s1 * y[ 0 ] - b1 * (  d.s2 * x[ 0 ] + d.s3 * x[ 0 ] );
	y[ 3 ] = n.s0 * x[ 3 ] + n.s1 * x[ 2 ] + n.s2 * x[ 1 ] + n.s3 * x[ 0 ]
		- d.s0 * y[ 2 ] - d.s1 * y[ 1 ] - d.s2 * y[ 0 ] - b1 * d.s3 * x[ 0 ];

	buffer2[ 0 ] = y[ 0 ];
	buffer2[ 1 ] = y[ 1 ];
	buffer2[ 2 ] = y[ 2 ];
	buffer2[ 3 ] = y[ 3 ];

    for (int i = 4; i < h; i++) {
        x[ 0 ] = x[ 1 ]; x[ 1 ] = x[ 2 ]; x[ 2 ] = x[ 3 ];
        x[ 3 ] = buffer[ i * stride ];
        yn = n.s0 * x[ 3 ] + n.s1 * x[ 2 ] + n.s2 * x[ 1 ] + n.s3 * x[ 0 ]
			 - d.s0 * y[ 3 ] - d.s1 * y[ 2 ] - d.s2 * y[ 1 ] - d.s3 * y[ 0 ];
		buffer2[ i ] = yn;
		y[ 0 ] = y[ 1 ]; y[ 1 ] = y[ 2 ]; y[ 2 ] = y[ 3 ]; y[ 3 ]= yn;
    }


    // reverse pass
	coord.y = h - 1;
	x[ 0 ] = buffer[ ( h - 1 ) * stride ];
	x[ 1 ] = buffer[ ( h - 2 ) * stride ];
	x[ 2 ] = buffer[ ( h - 3 ) * stride ];
	x[ 3 ] = buffer[ ( h - 4 ) * stride ];

	y[ 0 ] = m.s0 * x[ 0 ] + m.s1 * x[ 0 ] + m.s2 * x[ 0 ] + m.s3 * x[ 0 ]
		- b2 * ( d.s0 * x[ 0 ] + d.s1 * x[ 0 ] + d.s2 * x[ 0 ] + d.s3 * x[ 0 ] );
	y[ 1 ] = m.s0 * x[ 1 ] + m.s1 * x[ 0 ] + m.s2 * x[ 0 ] + m.s3 * x[ 0 ]
		- d.s0 * y[ 0 ] - b2 * ( d.s1 * x[ 0 ] + d.s2 * x[ 0 ] + d.s3 * x[ 0 ] );
	y[ 2 ] = m.s0 * x[ 2 ] + m.s1 * x[ 1 ] + m.s2 * x[ 0 ] + m.s3 * x[ 0 ]
		- d.s0 * y[ 1 ] - d.s1 * y[ 0 ] - b2 * (  d.s2 * x[ 0 ] + d.s3 * x[ 0 ] );
	y[ 3 ] = m.s0 * x[ 3 ] + m.s1 * x[ 2 ] + m.s2 * x[ 1 ] + m.s3 * x[ 0 ]
		- d.s0 * y[ 2 ] - d.s1 * y[ 1 ] - d.s2 * y[ 0 ] - b2 * d.s3 * x[ 0 ];

	write_imagef( output, coord,  buffer2[ h - 1 ] + y[ 0 ] );
	write_imagef( output, coord - ( int2 ) ( 0, 1 ),  buffer2[ h - 2 ] + y[ 1 ] );
	write_imagef( output, coord - ( int2 ) ( 0, 2 ),  buffer2[ h - 3 ] + y[ 2 ] );
	write_imagef( output, coord - ( int2 ) ( 0, 3 ),  buffer2[ h - 4 ] + y[ 3 ] );

    for (int i = h-5; i >= 0; i--) {
        x[ 0 ] = x[ 1 ]; x[ 1 ] = x[ 2 ]; x[ 2 ] = x[ 3 ];
        x[ 3 ] = buffer[ i * stride ];
		yn = m.s0 * x[ 3 ] + m.s1 * x[ 2 ] + m.s2 * x[ 1 ] + m.s3 * x[ 0 ]
			 - d.s0 * y[ 3 ] - d.s1 * y[ 2 ] - d.s2 * y[ 1 ] - d.s3 * y[ 0 ];
		coord.y = i;
		write_imagef( output, coord, buffer2[ i ] + yn );
		y[ 0 ] = y[ 1 ]; y[ 1 ] = y[ 2 ]; y[ 2 ] = y[ 3 ]; y[ 3 ]= yn;
    }
}
