# Details

Date : 2024-04-26 21:11:22

Directory /home/lkimuk/lkimuk-github/okec

Total : 50 files,  4208 codes, 1059 comments, 1461 blanks, all 6728 lines

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [README.md](/README.md) | Markdown | 1 | 0 | 0 | 1 |
| [examples/src/rf_discrete.cc](/examples/src/rf_discrete.cc) | C++ | 43 | 24 | 18 | 85 |
| [examples/src/wf_discrete.cc](/examples/src/wf_discrete.cc) | C++ | 41 | 24 | 20 | 85 |
| [examples/src/wf_net.cc](/examples/src/wf_net.cc) | C++ | 81 | 32 | 25 | 138 |
| [include/okec/algorithms/classic/default_decision_engine.h](/include/okec/algorithms/classic/default_decision_engine.h) | C++ | 31 | 0 | 19 | 50 |
| [include/okec/algorithms/classic/worse_fit_decision_engine.h](/include/okec/algorithms/classic/worse_fit_decision_engine.h) | C++ | 32 | 0 | 20 | 52 |
| [include/okec/algorithms/decision_engine.h](/include/okec/algorithms/decision_engine.h) | C++ | 74 | 0 | 41 | 115 |
| [include/okec/algorithms/machine_learning/DQN_decision_engine.h](/include/okec/algorithms/machine_learning/DQN_decision_engine.h) | C++ | 60 | 1 | 36 | 97 |
| [include/okec/algorithms/machine_learning/DQN_decision_engine2.h](/include/okec/algorithms/machine_learning/DQN_decision_engine2.h) | C++ | 0 | 59 | 34 | 93 |
| [include/okec/algorithms/machine_learning/RL_brain.hpp](/include/okec/algorithms/machine_learning/RL_brain.hpp) | C++ | 147 | 15 | 32 | 194 |
| [include/okec/common/message.h](/include/okec/common/message.h) | C++ | 51 | 1 | 25 | 77 |
| [include/okec/common/message_handler.hpp](/include/okec/common/message_handler.hpp) | C++ | 28 | 0 | 11 | 39 |
| [include/okec/common/resource.h](/include/okec/common/resource.h) | C++ | 63 | 3 | 40 | 106 |
| [include/okec/common/response.h](/include/okec/common/response.h) | C++ | 42 | 2 | 18 | 62 |
| [include/okec/common/task.h](/include/okec/common/task.h) | C++ | 71 | 1 | 38 | 110 |
| [include/okec/config/config.h](/include/okec/config/config.h) | C++ | 7 | 0 | 4 | 11 |
| [include/okec/devices/base_station.h](/include/okec/devices/base_station.h) | C++ | 93 | 1 | 41 | 135 |
| [include/okec/devices/client_device.h](/include/okec/devices/client_device.h) | C++ | 71 | 7 | 37 | 115 |
| [include/okec/devices/cloud_server.h](/include/okec/devices/cloud_server.h) | C++ | 31 | 5 | 20 | 56 |
| [include/okec/devices/edge_device.h](/include/okec/devices/edge_device.h) | C++ | 48 | 5 | 30 | 83 |
| [include/okec/network/multiple_LAN_WLAN_network_model.hpp](/include/okec/network/multiple_LAN_WLAN_network_model.hpp) | C++ | 122 | 20 | 32 | 174 |
| [include/okec/network/multiple_and_single_LAN_WLAN_network_model.hpp](/include/okec/network/multiple_and_single_LAN_WLAN_network_model.hpp) | C++ | 122 | 3 | 43 | 168 |
| [include/okec/network/network_model.hpp](/include/okec/network/network_model.hpp) | C++ | 185 | 28 | 47 | 260 |
| [include/okec/network/udp_application.h](/include/okec/network/udp_application.h) | C++ | 35 | 1 | 17 | 53 |
| [include/okec/okec.hpp](/include/okec/okec.hpp) | C++ | 26 | 1 | 9 | 36 |
| [include/okec/utils/color.h](/include/okec/utils/color.h) | C++ | 13 | 5 | 6 | 24 |
| [include/okec/utils/delegate.hpp](/include/okec/utils/delegate.hpp) | C++ | 38 | 0 | 16 | 54 |
| [include/okec/utils/format_helper.hpp](/include/okec/utils/format_helper.hpp) | C++ | 73 | 44 | 20 | 137 |
| [include/okec/utils/log.h](/include/okec/utils/log.h) | C++ | 43 | 0 | 15 | 58 |
| [include/okec/utils/message_helper.hpp](/include/okec/utils/message_helper.hpp) | C++ | 16 | 0 | 9 | 25 |
| [include/okec/utils/packet_helper.h](/include/okec/utils/packet_helper.h) | C++ | 16 | 2 | 11 | 29 |
| [include/okec/utils/read_csv.h](/include/okec/utils/read_csv.h) | C++ | 13 | 0 | 8 | 21 |
| [include/okec/utils/singleton.hpp](/include/okec/utils/singleton.hpp) | C++ | 24 | 25 | 11 | 60 |
| [include/okec/utils/visulizer.hpp](/include/okec/utils/visulizer.hpp) | C++ | 21 | 1 | 6 | 28 |
| [src/algorithms/classic/default_decision_engine.cc](/src/algorithms/classic/default_decision_engine.cc) | C++ | 283 | 40 | 66 | 389 |
| [src/algorithms/classic/worse_fit_decision_engine.cc](/src/algorithms/classic/worse_fit_decision_engine.cc) | C++ | 213 | 134 | 72 | 419 |
| [src/algorithms/decision_engine.cc](/src/algorithms/decision_engine.cc) | C++ | 309 | 30 | 55 | 394 |
| [src/algorithms/maching_learning/DQN_decision_engine.cc](/src/algorithms/maching_learning/DQN_decision_engine.cc) | C++ | 290 | 114 | 110 | 514 |
| [src/algorithms/maching_learning/DQN_decision_engine2.cc](/src/algorithms/maching_learning/DQN_decision_engine2.cc) | C++ | 0 | 299 | 76 | 375 |
| [src/common/message.cc](/src/common/message.cc) | C++ | 84 | 11 | 24 | 119 |
| [src/common/resource.cc](/src/common/resource.cc) | C++ | 180 | 8 | 47 | 235 |
| [src/common/response.cc](/src/common/response.cc) | C++ | 114 | 54 | 36 | 204 |
| [src/common/task.cc](/src/common/task.cc) | C++ | 325 | 7 | 68 | 400 |
| [src/devices/base_station.cc](/src/devices/base_station.cc) | C++ | 167 | 41 | 38 | 246 |
| [src/devices/client_device.cc](/src/devices/client_device.cc) | C++ | 118 | 4 | 23 | 145 |
| [src/devices/cloud_server.cc](/src/devices/cloud_server.cc) | C++ | 87 | 2 | 21 | 110 |
| [src/devices/edge_device.cc](/src/devices/edge_device.cc) | C++ | 113 | 3 | 24 | 140 |
| [src/network/udp_application.cc](/src/network/udp_application.cc) | C++ | 110 | 1 | 28 | 139 |
| [src/utils/packet_helper.cc](/src/utils/packet_helper.cc) | C++ | 26 | 0 | 9 | 35 |
| [src/utils/read_csv.cc](/src/utils/read_csv.cc) | C++ | 27 | 1 | 5 | 33 |

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)