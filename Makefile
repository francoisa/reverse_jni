CXX=g++
JAVA_LIB=/opt/java/jre/lib/amd64
JAVA_INC=/opt/java/include
OBJ=.obj
LIB=lib
CPPFLAGS =-g -Wall -fPIC -std=c++11 -I $(JAVA_INC) -I $(JAVA_INC)/linux 
CPPFLAGS+= -I /usr/include/cppunit/include
LDFLAGS=-g -L $(LIB) -L $(JAVA_LIB) -L $(JAVA_LIB)/server 
LDFLAGS+=-ljava -ljvm -lgtest -lpthread
CALC=antlrcalc
TEST=$(CALC).t
CALCLIB=lib$(CALC).so

$(TEST): $(OBJ)/$(TEST).o $(LIB)/$(CALCLIB)
	$(CXX) $< -l$(CALC) $(LDFLAGS) -o $@

$(OBJ)/%.o: %.cpp $(OBJ) $(LIB)
	$(CXX) -c $(CPPFLAGS) -o $@ $<

$(OBJ):
	@mkdir -p $(OBJ)

$(LIB):
	@mkdir -p $(LIB)

$(OBJ)/antlr_interface.o: antlr_interface.h antlr_interface.cpp

$(LIB)/$(CALCLIB): $(OBJ)/antlr_interface.o
	$(CXX) $^ -shared -Wl,-soname,$(CALCLIB) $(LDFLAGS) -o $(CALCLIB)
	mv $(CALCLIB) $(LIB)/$(CALCLIB)

clean:
	rm -f $(OBJ)/*.o $(TEST) $(LIB)/$(CALCLIB)

test:
	@source scripts/env; ./$(TEST)
