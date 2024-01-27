#include <iostream>
#include <thread>
#include "../LogicLib/Main.h"
#include <tacopie/tacopie>
#include <cpp_redis>
#include <mysql.h>
#include <string.h>
#include "../LogicLib/RankAndRdbmsUpdate.h"


// D:\Git\cpp_redis\includes\cpp_redis


int main()
{
	NLogicLib::Main main;
	NLogicLib::RankAndRdbmsUpdate update;
	main.Init(&update);

	
	////REDIS 예
	//cpp_redis::client client;

	//client.connect();

	//client.set("hello", "redis");
	//client.get("hello", [](cpp_redis::reply& reply) {
	//	std::cout << reply << std::endl;
	//	});
	////! std::future를 지원하기도 합니다.
	////! std::future<cpp_redis::reply> get_reply = client.get("hello");

	//client.sync_commit();
	std::thread rankUpdate([&]() {
		update.Run(); }
	);


	std::thread logicThread([&]() { 		
		main.Run(); }
	);
	
	std::cout << "press any key to exit...";
	getchar();

	main.Stop();
	logicThread.join();

	return 0;
}