/*
 * Copyright (c) 2014, webvariants GmbH, http://www.webvariants.de
 *
 * This file is released under the terms of the MIT license. You can find the
 * complete text in the attached LICENSE file or online at:
 *
 * http://www.opensource.org/licenses/mit-license.php
 *
 * @author: Thomas Krause (thomas.krause@webvariants.de)
 */

#ifndef __ENGINE_STARTER_COMPONENT__
#define __ENGINE_STARTER_COMPONENT__

#include "susi/world/BaseComponent.h"
#include "susi/enginestarter/EngineStarter.h"

namespace Susi {
    namespace EngineStarter {
        class StarterComponent : public Susi::System::BaseComponent , public Starter {
        public:
            StarterComponent( Susi::System::ComponentManager * mgr, std::string defaultPath = "" ) :
                Susi::System::BaseComponent {mgr}, Starter {}, _defaultPath{defaultPath} {}

            virtual void start() override {
                subscribe( std::string{"enginestarter::start"}, Susi::Events::Processor{[this]( Susi::Events::EventPtr evt ) {
                    handleStart( std::move( evt ) );
                }} );
                subscribe( std::string{"enginestarter::restart"}, Susi::Events::Processor{[this]( Susi::Events::EventPtr evt ) {
                    handleRestart( std::move( evt ) );
                }} );
                subscribe( std::string{"enginestarter::stop"}, Susi::Events::Processor{[this]( Susi::Events::EventPtr evt ) {
                    handleStop( std::move( evt ) );
                }} );

                subscribe( std::string{"global::start"}, Susi::Events::Processor{[this]( Susi::Events::EventPtr evt ) {
                    handleStart( std::move( evt ) );
                }} );
                subscribe( std::string{"global::restart"}, Susi::Events::Processor{[this]( Susi::Events::EventPtr evt ) {
                    handleRestart( std::move( evt ) );
                }} );
                subscribe( std::string{"global::stop"}, Susi::Events::Processor{[this]( Susi::Events::EventPtr evt ) {
                    handleStop( std::move( evt ) );
                }} );
                LOG(INFO) <<  "started EngineStarterComponent" ;
            }

            virtual void stop() override {
                unsubscribeAll();
                killall();
            }

            ~StarterComponent() {
                stop();
                LOG(INFO) <<  "stopped EngineStarterComponent" ;
            }
        protected:
            std::string _defaultPath;
            void handleStart( Susi::Events::EventPtr event );
            void handleRestart( Susi::Events::EventPtr event );
            void handleStop( Susi::Events::EventPtr event );
        };
    }

}

#endif // __ENGINE_STARTER_COMPONENT__
