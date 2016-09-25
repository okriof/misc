#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>


class Id
{
public: 
	Id(char c)
	try
	: mc(new char)
	{
		*mc = c;
		std::cout << "Generated object at " << (long) this << std::endl;
	}
	catch (...)
	{
		std::cout << "Could not allocate a char! Your computer has serious issues!" << std::endl;
		throw; // rethrow
	}

	
	~Id() 
	{
		delete mc;
		std::cout << "Destructed object at " << (long) this << std::endl; 
	}
	
	char get() const {return *mc;}
	Id(const Id& obj) : mc(new char) {
		*mc = *(obj.mc); 
		std::cout << "Copy-generated object at " << (long) this << std::endl;
	}

private:
	char* mc;
	const Id& operator=(const Id&); // not implemented
};




std::ostream& operator<<(std::ostream& os, const Id& id)
{
	os << id.get();
}




class Thingy
{
public:
	Thingy(char id = 'Q')
	try
	: id(id)
	{
		std::cout << "Doing thingy " << id << "... " << std::endl;
		if (rand() < RAND_MAX/10)
		{
			std::cout << "failed." << std::endl;
			throw "This is an exception!";
		}
		else std::cout << "OK!." << std::endl;
	}
	catch (...)
	{
		std::cout << "Something exceptional happened during construction, cant handle it here but some things could be taken care of if necessary." << std::endl;
	}


	Thingy(const Thingy& obj)
	: id(obj.id)
	{
		std::cout << "Copy-doing thingy " << id << "... " << std::endl;
		if (rand() < RAND_MAX/10)
		{
			std::cout << "failed." << std::endl;
			throw "This is an exception!";
		}
		else std::cout << "OK!." << std::endl;
	}



	~Thingy() { std::cout << "Undoing thingy " << id << std::endl; }

private:
	Id id;
};






int main()
{
	try {
		srand (time(NULL));
		std::vector<Thingy> myThingies(20);		

/*		Thingy a('A');
		Thingy b('B');
		Thingy c('C');
		Thingy d('D');
		Thingy e('E');
		Thingy f('F');
		Thingy g('G');
		Thingy h('H');
		Thingy i('I');
		Thingy j('J');
		Thingy k('K');
		Thingy l('L');
		Thingy m('M');
		Thingy n('N');
		Thingy o('O'); */

		// do stuff with all thingies


	}
	catch (const char* e)
	{
		std::cout << "Caught: " << e << std::endl;
	}
}
