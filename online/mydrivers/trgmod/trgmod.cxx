#include "trgmod.h"


int TRG::trgmod_init(char *trgIP) {
  
  std::string ip_raw(trgIP);
  SERVER_ADDRESS = "tcp://" + ip_raw+":1883";
  //std::cout<<SERVER_ADDRESS<<std::endl;
  
  int ret = initializeFPGA();
  //ret += configureFPGA();

  FPGAinit = true;
  return ret;
}

int TRG::trgmod_config(int *trgreg) {
  int ret  = 0 ;
  //if(!FPGAinit) {
  //  ret += initializeFPGA();
  //}
  ret += configureFPGA(trgreg);

  if(ret==0) {
    for(int i=0; i<LENGTH_REG; i++) {
      CONF[i] = trgreg[i];
    }
  }
  
  return ret;
}





int TRG::trgmod_read() {
  int stat = 0;
  //if(!FPGAinit) {
  //  stat = initializeFPGA();
  //}
  //if(stat!=0) {
  //  throw std::runtime_error("Unable to initialize FPGA of the Trigger Module.");
  //}
  
  mqtt::client cli(SERVER_ADDRESS, CLIENT_CONSUME_ID);

  auto connOpts = mqtt::connect_options_builder()
    //.user_name("user")
    //.password("passwd")
    .keep_alive_interval(std::chrono::seconds(30))
    .automatic_reconnect(std::chrono::seconds(2), std::chrono::seconds(30))
    .clean_session(false)
    .finalize();

  // You can install a callback to change some connection data
  // on auto reconnect attempts. To make a change, update the
  // `connect_data` and return 'true'.
  /*cli.set_update_connection_handler(
				    [](mqtt::connect_data& connData) {
				      std::string newUserName { "newuser" };
				      if (connData.get_user_name() == newUserName)
					return false;
				      
				      //cout << "Previous user: '" << connData.get_user_name()
				      //   << "'" << endl;
				      connData.set_user_name(newUserName);
				      //cout << "New user name: '" << connData.get_user_name()
				      //   << "'" << endl;
				      return true;
				    }
				    );*/

  const std::vector<std::string> TOPICS { CONF_TOPIC};
  const std::vector<int> vqos { QOS };

  try {
    mqtt::connect_response rsp = cli.connect(connOpts);

    if (!rsp.is_session_present()) {
      cli.subscribe(TOPICS, vqos);
    }
    // Consume messages

    while (true) {
      auto msg = cli.consume_message();

      if (msg) {
	std::string topic = msg->get_topic();
	std::string payload = msg->to_string();
	if (topic == CONF_TOPIC &&
	    (payload).find("ActiveCONF") != std::string::npos) {

	  std::stringstream sspayload;
	  sspayload << payload;
	  
	  std::string tmp;
	  int counter = 0 ;
	  while(std::getline(sspayload, tmp, ' ')) {
	    if(counter!=0 && counter <=8) {
	      CONF[counter-1] = std::stoi(tmp);
	    }
	    counter++;
	  }
	  //cout << "Exit command received" << endl;
	  break;
	}
      }
      
      else if (!cli.is_connected()) {
	while (!cli.is_connected()) {
	  std::this_thread::sleep_for(std::chrono::milliseconds(1)); ///TO BE UNDERSTOOD
	}
      }
    }
    cli.disconnect();
  }
  catch (const mqtt::exception& exc) {
    std::cerr << exc.what() << std::endl;
    return 1;
  }

  return 0;
}









int TRG::initializeFPGA() {
  trgnamespace::sample_mem_persistence persist;
  mqtt::client client(SERVER_ADDRESS, CLIENT_PUBLISH_ID, &persist);

  trgnamespace::user_callback cb;
  client.set_callback(cb);

  mqtt::connect_options connOpts;
  connOpts.set_keep_alive_interval(20);
  connOpts.set_clean_session(true);

  try {
    client.connect(connOpts);

    auto pubmsg = mqtt::make_message(SETUP_TOPIC, "start triggerModule");
    pubmsg->set_qos(QOS);
    client.publish(pubmsg);

    client.disconnect();
  }
  catch (const mqtt::persistence_exception& exc) {
    std::cerr << "Persistence Error: " << exc.what() << " ["
              << exc.get_reason_code() << "]" << std::endl;
    return 1;
  }
  catch (const mqtt::exception& exc) {
    std::cerr << exc.what() << std::endl;
    return 1;
  }

  std::this_thread::sleep_for(std::chrono::seconds(2));
  

  return 0;
}

int TRG::configureFPGA(int* trgreg) {
  trgnamespace::sample_mem_persistence persist;
  mqtt::client client(SERVER_ADDRESS, CLIENT_PUBLISH_ID, &persist);

  trgnamespace::user_callback cb;
  client.set_callback(cb);

  mqtt::connect_options connOpts;
  connOpts.set_keep_alive_interval(20);
  connOpts.set_clean_session(true);


  std::string configuration = "loadusrconf ";
  for(int i=0; i<LENGTH_REG; i++) {
    configuration += std::to_string(trgreg[i]);
    configuration += " ";
  }
  
  try {
    client.connect(connOpts);

    auto pubmsg = mqtt::make_message(SETUP_TOPIC, configuration);
    pubmsg->set_qos(QOS);
    client.publish(pubmsg);

    client.disconnect();
  }
  catch (const mqtt::persistence_exception& exc) {
    std::cerr << "Persistence Error: " << exc.what() << " ["
              << exc.get_reason_code() << "]" << std::endl;
    return 1;
  }
  catch (const mqtt::exception& exc) {
    std::cerr << exc.what() << std::endl;
    return 1;
  }

  std::this_thread::sleep_for(std::chrono::seconds(8));
  

  return 0;
}
