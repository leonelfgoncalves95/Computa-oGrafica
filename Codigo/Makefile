linux:
	g++ -w -o gerador gerador.cpp
	g++ motor.cpp -lglut -lGLU -lGL -lGLEW -lIL tinyxml/tinystr.cpp tinyxml/tinyxml.cpp tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp -w -o motor
	./gerador sphere 1 22 22 sphere.3d
	./gerador sphere 1 12 12 sphere_light.3d
	./gerador sphere 1 4 4 sphere_lightest.3d
	./motor solar_system.xml

macOS:
	g++ -w -o gerador gerador.cpp
	g++ -framework OpenGL -framework GLUT -lil -lilu -lilut tinyxml/tinystr.cpp tinyxml/tinyxml.cpp tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp -w -g -o motor motor.cpp
	./gerador sphere 1 22 22 sphere.3d
	./gerador sphere 1 12 12 sphere_light.3d
	./gerador sphere 1 4 4 sphere_lightest.3d
	./motor solar_system.xml

delete:
	rm plane.3d box.3d cone.3d sphere.3d sphere_light.3d sphere_lightest.3d teapot.3d gerador motor