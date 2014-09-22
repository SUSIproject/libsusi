#include "util/ComponentTest.h"

class ApiServerComponentTest : public ComponentTest {
protected:
	std::shared_ptr<Susi::Api::ApiServerComponent> apiserver;
	bool called{false};
	std::mutex mutex;
	std::condition_variable cond;

	std::string sessionID{"123"};

	void waitForCondition(int timeout){
		std::unique_lock<std::mutex> lock{mutex};
		cond.wait_for(lock,std::chrono::milliseconds{timeout},[this](){return called;});
		EXPECT_EQ(true,called);
		called = false;
	}

	virtual void SetUp() override {
		componentManager->startComponent("apiserver");
		apiserver = componentManager->getComponent<Susi::Api::ApiServerComponent>("apiserver");
	}
	
	virtual void TearDown() override {
		componentManager->stopComponent("apiserver");
	}

	virtual void GoodCases() override {
		apiserver->registerSender(sessionID,[this](Susi::Util::Any & packet){
			Susi::Util::Any expect = Susi::Util::Any::Object{
				{"type","status"},
				{"error",false}
			};
			EXPECT_EQ(expect.toJSONString(),packet.toJSONString());
			called = true;
			cond.notify_one();
		});
		apiserver->onConnect(sessionID);

		Susi::Util::Any msg = Susi::Util::Any::Object{
			{"type","registerConsumer"},
			{"data","sample"}
		};
		apiserver->onMessage(sessionID,msg);
		waitForCondition(250);

		msg = Susi::Util::Any::Object{
			{"type","registerProcessor"},
			{"data","sample"}
		};
		apiserver->onMessage(sessionID,msg);
		waitForCondition(250);

		msg = Susi::Util::Any::Object{
			{"type","publish"},
			{"data",Susi::Util::Any::Object{
				{"topic","sample"}
			}}
		};

		Susi::Util::Any procEvent;

		apiserver->registerSender(sessionID,[this,&procEvent](Susi::Util::Any & packet){
			if(packet["type"]=="status"){
				Susi::Util::Any expect = Susi::Util::Any::Object{
					{"type","status"},
					{"error",false}
				};
				EXPECT_EQ(expect.toJSONString(),packet.toJSONString());
				Susi::Logger::debug("in status");
			}else if(packet["type"]=="ack"){
				EXPECT_EQ(Susi::Util::Any{"sample"},packet["data"]["topic"]);
				EXPECT_EQ(Susi::Util::Any{"123"},packet["data"]["sessionid"]);
				EXPECT_EQ(size_t{0},packet["data"]["headers"].size());
				Susi::Logger::debug("in ack");
			}else if(packet["type"]=="consumerEvent"){
				EXPECT_EQ(Susi::Util::Any{"sample"},packet["data"]["topic"]);
				EXPECT_EQ(Susi::Util::Any{"123"},packet["data"]["sessionid"]);
				EXPECT_EQ(size_t{0},packet["data"]["headers"].size());
				Susi::Logger::debug("in consumer");
			}else if(packet["type"]=="processorEvent"){
				EXPECT_EQ(Susi::Util::Any{"sample"},packet["data"]["topic"]);
				EXPECT_EQ(Susi::Util::Any{"123"},packet["data"]["sessionid"]);
				EXPECT_EQ(size_t{0},packet["data"]["headers"].size());
				Susi::Logger::debug("in processor");
				procEvent = packet;
			}
			called = true;
			cond.notify_one();
		});
		apiserver->onMessage(sessionID,msg);
		waitForCondition(250);//status
		waitForCondition(250);//processorEvent
		procEvent["type"] = "ack";
		apiserver->onMessage(sessionID,procEvent);
		waitForCondition(250);//status
		waitForCondition(250);//consumerEvent
		waitForCondition(250);//ack
		apiserver->onClose(sessionID);
	}

	virtual void BadCases() override {

	}

	virtual void EdgeCases() override {

	}

};

TEST_F(ApiServerComponentTest,GoodCases){
	GoodCases();
}

TEST_F(ApiServerComponentTest,BadCases){
	BadCases();
}

TEST_F(ApiServerComponentTest,EdgeCases){
	EdgeCases();
}
