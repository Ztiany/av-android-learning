#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D uTextureY;
uniform sampler2D uTextureU;
uniform sampler2D uTextureV;

varying vec4 vTexcoord;

// YUV420 to RGB conversion
vec3 YUV420ToRGB(float y, float u, float v){
    return vec3(
    y+1.402*v,
    y-0.34413*u-0.71414*v,
    y+1.772*u
    );
}

// YUV420 to RGB conversion via matrix multiplication
vec3 YUV420ToRGBViaMatrix(float y, float u, float v){
    mat3 transform_matrix=mat3(
    1.0, 1.0, 1.0,
    0.0, -0.34413, 1.772,
    1.402, -0.71414, 0.0
    );
    return transform_matrix*vec3(y, u, v);
}

void main(){
    float Y=texture2D(uTextureY, vTexcoord.xy).a;
    float U=texture2D(uTextureU, vTexcoord.xy).a-0.5;
    float V=texture2D(uTextureV, vTexcoord.xy).a-0.5;

    gl_FragColor=vec4(YUV420ToRGBViaMatrix(Y, U, V), 1.0);
}