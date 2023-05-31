todo: spinningcube_withlight spinningcube_withlight_SKEL

LDLIBS=-lGL -lGLEW -lglfw -lm 

spinningcube_withlight: spinningcube_withlight.o textfile.o
spinningcube_withlight_SKEL: spinningcube_withlight_SKEL.o textfile.o

clean:
	rm -f *.o *~

cleanall: clean
	rm -f spinningcube_withlight
