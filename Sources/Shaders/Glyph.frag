#include "evk.frag"

PUSH(
    vec2 ScreenSize;
    vec2 Position;
    vec2 Scale;
    float Time;
    int GlyphsBufferRID;
    int OffsetCoords;
    int NumCoords;
)

BINDING_BUFFER_RW(GlyphsBuffer,
    vec2 at[];
)

IN(0) vec2 UV;
OUT(0) vec4 out_Color;

vec2 Bezier2(vec2 a, vec2 b, vec2 c, float t) {
    vec2 s1 = a*(1.0-t) + b*t;
    vec2 s2 = b*(1.0-t) + c*t;
    return t*t*(a - 2*b + c) + t*2*(b - a) + a;
}
// Projection of b onto a.
vec2 proj(vec2 a, vec2 b) {
    return normalize(a) * dot(a,b) / length(a);
}
vec2 orth(vec2 a, vec2 b) {
    return b - proj(a, b);
}
float dot2(vec2 v) {
    return dot(v,v);
}
bool lineTest(vec2 p, vec2 A, vec2 B) {
    int cs = int(A.y < p.y) * 2 + int(B.y < p.y);
    if(cs == 0 || cs == 3) {
        return false;
    } // trivial reject

    vec2 v = B - A;

    // Intersect line with x axis.
    float t = (p.y-A.y)/v.y;

    return (A.x + t*v.x) > p.x;
}
bool bezierTest(vec2 p, vec2 A, vec2 B, vec2 C) {
    // Compute barycentric coordinates of p.
    // p = s * A + t * B + (1-s-t) * C
    vec2 v0 = B - A, v1 = C - A, v2 = p - A;
    float det = v0.x * v1.y - v1.x * v0.y;
    float s = (v2.x * v1.y - v1.x * v2.y) / det;
    float t = (v0.x * v2.y - v2.x * v0.y) / det;

    if(s < 0 || t < 0 || (1-s-t) < 0) {
        return false; // outside triangle
    }

    // Transform to canonical coordinte space.
    float u = s * .5 + t;
    float v = t;

    return u*u < v;
}
float sdLine(vec2 p, vec2 a, vec2 b) {
    float d = length(orth(b-a, p-a));
    mat2 M = mat2(b-a, p-a);
    return determinant(M) >= 0 ? d : -d;
}
float sdBezier2(vec2 uv, vec2 p0, vec2 p1, vec2 p2) {
    const mat2 trf1 = mat2(vec2(-1, 2), vec2(1, 2));
    mat2 M = mat2(p0-p1, p2-p1);
    if (determinant(M) == 0.0) {
        return sdLine(uv, p0, p2);
    }

    mat2 trf2 = inverse(M);
    mat2 trf=trf1*trf2;

    uv-=p1;
    vec2 xy=trf*uv;
    xy.y-=1.;

    vec2 gradient;
    gradient.x=2.*trf[0][0]*(trf[0][0]*uv.x+trf[1][0]*uv.y)-trf[0][1];
    gradient.y=2.*trf[1][0]*(trf[0][0]*uv.x+trf[1][0]*uv.y)-trf[1][1];

    return (xy.x*xy.x-xy.y)/length(gradient);
}
float sdSegment(vec2 p, vec2 a, vec2 b ) {
    vec2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}
float udBezier(vec2 pos, vec2 A, vec2 B, vec2 C ) {
    // Are the points collinear?
    mat2 M = mat2(C-A, B-A);
    if (abs(determinant(M)) < 0.05) {
        return sdSegment(pos, A, C);
    }

    vec2 a = B - A;
    vec2 b = A - 2.0*B + C;
    vec2 c = a * 2.0;
    vec2 d = A - pos;
    float kk = 1.0/dot(b,b);
    float kx = kk * dot(a,b);
    float ky = kk * (2.0*dot(a,a)+dot(d,b)) / 3.0;
    float kz = kk * dot(d,a);
    float res = 0.0;
    float p = ky - kx*kx;
    float p3 = p*p*p;
    float q = kx*(2.0*kx*kx-3.0*ky) + kz;
    float h = q*q + 4.0*p3;
    if( h >= 0.0) {
        h = sqrt(h);
        vec2 x = (vec2(h,-h)-q)/2.0;
        vec2 uv = sign(x)*pow(abs(x), vec2(1.0/3.0));
        float t = clamp( uv.x+uv.y-kx, 0.0, 1.0 );
        res = dot2(d + (c + b*t)*t);
    } else {
        float z = sqrt(-p);
        float v = acos( q/(p*z*2.0) ) / 3.0;
        float m = cos(v);
        float n = sin(v)*1.732050808;
        vec3  t = clamp(vec3(m+m,-n-m,n-m)*z-kx,0.0,1.0);
        res = min( dot2(d+(c+b*t.x)*t.x),
                   dot2(d+(c+b*t.y)*t.y) );
        // the third root cannot be the closest
        // res = min(res,dot2(d+(c+b*t.z)*t.z));
    }
    return sqrt( res );
}

void main() {
    vec4 overlay = vec4(0);
    out_Color = vec4(vec3(0.3),1);
    
    float d = 100000.0;
    float s = 1.0;

    for(int i = OffsetCoords; i < OffsetCoords+NumCoords; i+=3) {
        vec2 p1 = GlyphsBuffer[GlyphsBufferRID].at[i+0]*Scale+Position;
        vec2 p2 = GlyphsBuffer[GlyphsBufferRID].at[i+1]*Scale+Position;
        vec2 p3 = GlyphsBuffer[GlyphsBufferRID].at[i+2]*Scale+Position;
        
        d = min(d, udBezier(UV, p1, p2, p3));
        if(lineTest(UV, p1, p3)) {
            s = -s;
        }
        if(bezierTest(UV, p1, p2, p3)) {
            s = -s;
        }

        // vec4 c = vec4(fract(vec3(0.3,0.4,0.5)*i+0.3),1);
        // for(float t = 0; t < 1.0; t += 0.02) if(length(Bezier2(p1-UV, p2-UV, p3-UV, t)) <  0.002)overlay=c;
        // if(abs(length(p1-UV)-0.005) <  0.001)overlay=c;
        // if(abs(length(p2-UV)-0.005) <  0.001)overlay=c;
        // if(abs(length(p3-UV)-0.005) <  0.001)overlay=c;
    }

    float alpha = 1.0-clamp(s*d+0.5, 0.0, 1.0);
    out_Color = vec4(vec3(mix(0.0, 1.0, alpha)), 1.0);

    if(overlay.w != 0) {
        out_Color = overlay;
    }
}