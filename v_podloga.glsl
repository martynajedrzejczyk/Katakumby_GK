#version 330

//Zmienne jednorodne
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

//Atrybuty
in vec4 vertex; //wspolrzedne wierzcholka w przestrzeni modelu
in vec4 color; //kolor zwi¹zany z wierzcho³kiem
in vec4 normal; //wektor normalny w przestrzeni modelu
in vec2 texCoord0;

//Zmienne interpolowane
out vec4 ic;
out vec4 l;
out vec4 n;
out vec4 v;
out float d;
out vec2 iTexCoord0;

out vec4 ic2;
out vec4 l2;
out vec4 n2;
out vec4 v2;
out float d2;
out vec2 iTexCoord2;


void main(void) {

    vec4 lp = vec4(0, 0, 0, 1); //przestrzeñ œwiata

    l = normalize(lp - V*M*vertex); //wektor do œwiat³a w przestrzeni oka
    v = normalize(lp - V * M * vertex); //wektor do obserwatora w przestrzeni oka
    n = normalize(V * M * normal); //wektor normalny w przestrzeni oka
    iTexCoord0 = texCoord0;

    d = distance(V*M*vertex, lp);
    d = clamp(1 - d/10, 0, 1);
    ic = color;




    //dodatkowe swiatlo
    vec4 swiatlo = vec4(3.94158, 3.5, 14.8518, 1.0);

    l2 = normalize(swiatlo - M*vertex); //wektor do œwiat³a w przestrzeni oka
    v2 = normalize(swiatlo - M * vertex); //wektor do obserwatora w przestrzeni oka
    n2 = normalize(M * normal); //wektor normalny w przestrzeni oka
    iTexCoord2 = texCoord0;

    d2 = distance(M*vertex, swiatlo);
    d2 = clamp(1 - d2/3, 0, 1);
    
    gl_Position=P*V*M*vertex;
}