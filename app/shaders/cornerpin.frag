// Input texture
uniform sampler2D ove_maintex;
uniform sampler2D tex_in;
uniform bool perspective_in;

// Input texture coordinate
varying vec2 ove_texcoord;

varying vec2 q;
varying vec2 b1;
varying vec2 b2;
varying vec2 b3;

float Wedge2D(vec2 v, vec2 w) {
	return (v.x*w.y) - (v.y*w.x);
}

void main() {
    if(perspective_in){
        gl_FragColor = texture2D(tex_in, ove_texcoord);
    } else {
	    float A = Wedge2D(b2, b3);
		float B = Wedge2D(b3, q) - Wedge2D(b1, b2);
		float C = Wedge2D(b1, q);

		vec2 uv;

		// solve for v
		if (abs(A) < 0.001) {
			uv.y = -C/B;
		} else {
			float discrim = B*B - 4.0*A*C;
			uv.y = 0.5 * (-B + sqrt(discrim)) / A;
		}

		// solve for u
		vec2 denom = b1 + uv.y * b3;
		if (abs(denom.x) > abs(denom.y)) {
			uv.x = (q.x - b2.x * uv.y) / denom.x;
		} else {
			uv.x = (q.y - b2.y * uv.y) / denom.y;
		}

		uv.y = 1.0 - uv.y;

		gl_FragColor = texture2D(tex_in, uv);
	}
}
