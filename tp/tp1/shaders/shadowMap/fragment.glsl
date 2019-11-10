#version 330

out vec4 fragment_color;

void main(){
    //fragmentdepth = gl_FragCoord.z;

	//fragment_color = vec4(gl_FragCoord.z,gl_FragCoord.z,gl_FragCoord.z,1);
	fragment_color = vec4(gl_FragCoord.z,gl_FragCoord.z,gl_FragCoord.z,1);

}