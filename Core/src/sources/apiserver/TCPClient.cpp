#include "susi/apiserver/TCPClient.h"



Susi::Api::TCPClient::TCPClient( std::string address ) : sa {address} {
    startRunloop();
}

Susi::Api::TCPClient::TCPClient( TCPClient && other ) {
    isClosed.store(other.isClosed.load());
    std::swap( sa,other.sa );
    std::swap( sock,other.sock );
    std::swap( runloop,other.runloop );
}


void Susi::Api::TCPClient::close() {
    if( !isClosed.load() ) {
        isClosed.store(true);
        try {
            sock.shutdown();
            sock.close();
            join();
            onClose();
        }
        catch( const std::exception & e ) {
            //std::cout<<"error while closing/joining: "<<e.what()<<std::endl;
            onClose();
        }
    }
}
void Susi::Api::TCPClient::send( std::string msg ) {
    try{
        sock.sendBytes( msg.c_str(),msg.size() );
    }catch(std::exception & e){
        LOG(DEBUG) << "error while sending: "<<e.what();
        //sendbuff += msg;
    }
}
void Susi::Api::TCPClient::join() {
    if(runloop.joinable())runloop.join();
}
Susi::Api::TCPClient::~TCPClient() {
    close();
    join();
}

void Susi::Api::TCPClient::startRunloop(){
    isClosed.store(false);
    runloop = std::move( std::thread{
        [this]() {
            char buff[1024];
            int bs;
            bool onCloseCalled = false;
            while( !(isClosed.load()) ) { // data chunk loop
                try {
                    bs = sock.receiveBytes( buff,1024 );
                    if( bs<=0 ) {
                        throw std::runtime_error{"got zero bytes -> connection reset by peer"};
                    }
                    std::string data {buff,static_cast<size_t>( bs )};
                    onData( data );
                }
                catch( const Poco::TimeoutException & e ) {}
                catch( const std::exception & e ) {
                    if(!onCloseCalled){
                        onCloseCalled = true;
                        onClose();
                    }
                    try{
                        sock.close();
                        sock.connect(sa);
                        sock.setReceiveTimeout( Poco::Timespan{0,100000} );
                        onConnect();
                        onReconnect();
                        onCloseCalled = false;
                    }catch(const Poco::Net::ConnectionRefusedException & e){
                        usleep(200000);
                    }
                    /*size_t retryCount = 0;
                    bool success = false;
                    while(!(isClosed.load()) && (retryCount < maxReconnectCount)){
                        LOG(DEBUG)<<"try reconnect...";
                        try{
                            retryCount++;
                            sock.close();
                            sock.connect(sa);
                            onReconnect();
                            success = true;
                            if(sendbuff != ""){
                                send(sendbuff);
                                sendbuff = "";
                            }
                            break;
                        }catch(const std::exception & e){
                            LOG(DEBUG) << "Error in reconnect: "<<e.what();
                        }               
                        usleep(500000);
                        //std::this_thread::sleep_for(std::chrono::milliseconds{500});
                    }
                    if(!success){
                        LOG(DEBUG) << "no success with reconnecting, finally closing...";
                        LOG(DEBUG) << "maxReconnectCount was: "<<maxReconnectCount;
                        LOG(DEBUG) << "isClosed was: "<<isClosed.load();
                        onClose();
                        break;
                    }else{
                        LOG(DEBUG) << "reconnected tcp client...";
                    }*/
                }
            }
        }
    } );
}
