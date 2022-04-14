#version 330

uniform sampler2D textureMap0;
uniform sampler2D textureMap1;

out vec4 pixelColor; //Zmienna wyjsciowa fragment shadera. Zapisuje sie do niej ostateczny (prawie) kolor piksela

in vec4 ic; 
in vec4 n;
in vec4 l;
in vec4 v;
in float d;
in vec2 iTexCoord0;

in vec4 ic2; 
in vec4 n2;
in vec4 l2;
in vec4 v2;
in float d2;
in vec2 iTexCoord2;

void main(void) {

        //vec4 swiatlo = vec4(3.5f, 7.5f, 7.0f, 1.0f);
    //3.94158,3.5,14.8518
    vec4 swiatlo = vec4(3.94158f, 3.5f, 14.8518f, 1.0f);
    //swiatlo
    vec4 ml = normalize(l2);
    vec4 mn = normalize(n2);
    vec4 mv = normalize(v2);
    //Wektor odbity
    vec4 mr = reflect(-ml, mn);

    //Parametry powierzchni
    vec4 kd = texture(textureMap0, iTexCoord2); 
    vec4 ks = texture(textureMap1, iTexCoord2);

    //Obliczenie modelu oœwietlenia
    float nl = clamp(dot(mn, ml), 0, 1);
    float rv = pow(clamp(dot(mr, mv), 0, 1),25);
    pixelColor = vec4(kd.rgb * nl * d2, kd.a) + vec4(ks.rgb*rv* d2, 0);




    //latarka
    //Znormalizowane interpolowane wektory
    ml = normalize(l);
    mn = normalize(n);
    mv = normalize(v);
    //Wektor odbity
    mr = reflect(-ml, mn);

    //Parametry powierzchni
    kd = texture(textureMap0, iTexCoord0); 
    ks = texture(textureMap1, iTexCoord0);

    //Obliczenie modelu oœwietlenia
    nl = clamp(dot(mn, ml), 0, 1);
    rv = pow(clamp(dot(mr, mv), 0, 1),25);
    pixelColor = pixelColor + vec4(kd.rgb * nl * d, kd.a) + vec4(ks.rgb*rv* d, 0);
}