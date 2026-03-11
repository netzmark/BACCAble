/*
 * processingMessage0x00000226.c
 *
 *  Created on: May 3, 2025
 *      Author: GauchoHP
 */

#include "processingExtendedMessage.h"

void processingExtendedMessage(){
	#if defined(C1baccable)
		if(immobilizerEnabled && (engineOnSinceMoreThan5seconds<500)){ //if immo enabled and engine is off
			//if it is a message of connection to RFHUB, reset the connection periodically, but start the panic alarm only once
			if(floodTheBus==0){ //if we are not flooding the bus
				uint8_t responseOffset=rx_msg_data[0]>>4; //0=single frame , 1=first fragmented frame 2=fragmented frame, 3=frame ack
				if((rx_msg_header.ExtId & 0xFFFFFFF0)==0x18DAC7F0){ 		//if it is message from the thief
					if(responseOffset<2){ //we pass this if, in case of single frame and first fragmented frame
						switch(rx_msg_data[responseOffset+1]){
							case 0x10: //diagnostic session
							case 0x27: //security access
							case 0x29: //authentication
							case 0x3E: //tester presence
							//case 0x1A: //??
							case 0x2E: //write data by identifier
							case 0x3D: //write memory by address
								floodTheBus=1; //reset the RFHUB and start the alarm
								break;
							default:
								break;
						}
					}
				}else if((rx_msg_header.ExtId & 0xFFFFF0FF)==0x18DAF0C7) { 	//if it is a reply from rfhub
					//if(floodTheBusStartTime==0){ //this allows to read rfhub messages only if it was the first time
						if(responseOffset<2){ //we pass this if, in case of single frame and first fragmented frame
							switch(rx_msg_data[responseOffset+1]){
								case 0x50: //diagnostic session	//
								case 0x67: //security access
								case 0x69: //authentication
								case 0x7E: //tester presence 	//
								//case 0x1A: //??
								case 0x6E: //write data by identifier
								case 0x7D: //write memory by address
									floodTheBus=1; //reset the RFHUB and start the alarm
									break;
								default:
									break;
							}
						}
					//}
				}
				if(floodTheBus==1){ //if we engaged the immobilizer
					floodTheBusStartTime=currentTime; //set initial time we started to flood the bus
					onboardLed_blue_on(); //light a led
				}

			}
		} //end of immobilizer section

//	ori code with hardcoded read offset from D4-D7 only, negative offsets are not possible (if needed is some case)
		if ((rx_msg_header.ExtId==single_uds_params_array[uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]].replyId) && baccableDashboardMenuVisible){ //if we received UDS message with current selected parameter, let's aquire it
			if(dashboard_menu_indent_level==1 && main_dashboardPageIndex==1){ //if we are in show params menu
				onboardLed_blue_on();
			  if (rx_msg_header.DLC>=4+single_uds_params_array[uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]].replyOffset+single_uds_params_array[uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]].replyLen){
					uint8_t numberOfBytesToRead=single_uds_params_array[uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]].replyLen;
					// Limita il numero di byte a un massimo di 4 per evitare overflow
					if (numberOfBytesToRead > 4) {
						numberOfBytesToRead = 4;
					}
					uint32_t tmpVal=0; //take value of received parameter

					// Costruisce il valore a partire dai byte ricevuti
					for (size_t i = 0; i < numberOfBytesToRead; i++) {
						tmpVal |= ((uint32_t)rx_msg_data[4+single_uds_params_array[uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]].replyOffset+i]) << (8 * (numberOfBytesToRead - 1 - i));
					}

					tmpVal+=single_uds_params_array[uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]].replyValOffset;
					float tmpVal2 =tmpVal * single_uds_params_array[uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]].replyScale;
					tmpVal2 +=single_uds_params_array[uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]].replyScaleOffset;

					if(uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]== uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[!currentParamElementSelection]){
						currentParamElementSelection=0; //single param
					}

					dashboardParamCouple[currentParamElementSelection]=tmpVal2;//aquire param in a variabile
					sendDashboardPageToSlaveBaccable();//send parameters to BH
				}
			}
		}


//	//	@netzmark: corrected code for negative offsets and very frames (like the ones having 3 bytes only), not fully tested  - for future requirements
//	//	@netzmark: requires change "uint8_t replyOffset" to int type in uds_parameters.h
//		if ((rx_msg_header.ExtId==single_uds_params_array[uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]].replyId) && baccableDashboardMenuVisible){ //if we received UDS message with current selected parameter, let's aquire it
//			if(dashboard_menu_indent_level==1 && main_dashboardPageIndex==1){ //if we are in show params menu
//				onboardLed_blue_on();
//			/* Relaxed DLC check to support short CAN frames (DLC < 8) and prevent rejecting valid UDS responses or broadcasts that end exactly on the data byte.
//			 * The fix for uds specific nessages receive
//			 * */
//				if (rx_msg_header.DLC>=4+single_uds_params_array[uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]].replyOffset){
//					uint8_t numberOfBytesToRead=single_uds_params_array[uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]].replyLen;
//					// Limita il numero di byte a un massimo di 4 per evitare overflow
//					if (numberOfBytesToRead > 4) {
//						numberOfBytesToRead = 4;
//					}
//					uint32_t tmpVal=0; //take value of received parameter
//
//					// Costruisce il valore a partire dai byte ricevuti
//					for (int i = 0; i < numberOfBytesToRead; i++) {
//						// @netzmark: This cast to (int) ensures that the STM32 correctly calculates negative offsets, like -1 or -4, preventing the processor from accessing invalid memory addresses and crashing.to serve negative offsets for short frames
//
//					    int targetIdx = 4 + (int)single_uds_params_array[uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]].replyOffset + i;
//					    if (targetIdx >= 0 && targetIdx < (int)rx_msg_header.DLC) {
//					        tmpVal |= ((uint32_t)rx_msg_data[targetIdx]) << (8 * (numberOfBytesToRead - 1 - i));
//					    }
//					}
//
//					tmpVal+=single_uds_params_array[uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]].replyValOffset;
//					float tmpVal2 =tmpVal * single_uds_params_array[uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]].replyScale;
//					tmpVal2 +=single_uds_params_array[uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]].replyScaleOffset;
//
//					if(uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]== uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[!currentParamElementSelection]){
//						currentParamElementSelection=0; //single param
//					}
//
//					dashboardParamCouple[currentParamElementSelection]=tmpVal2;//aquire param in a variabile
//					sendDashboardPageToSlaveBaccable();//send parameters to BH
//				}
//			}
//		}

//	// @netzmark: corrected simplified code for negative offsetsand very frames (like the ones having 3 bytes only), not tested at all tested - for future requirements
//	//	@netzmark: requires change "uint8_t replyOffset" to int type in uds_parameters.h

//		// 1. Create a pointer to the current parameter definition to simplify access and improve readability
//		const uds_param_single_element *p = &single_uds_params_array[uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection]];
//
//		// Check if the received CAN ID matches the expected replyId and the dashboard is visible
//		if ((rx_msg_header.ExtId == p->replyId) && baccableDashboardMenuVisible) {
//		    // Ensure we are in the correct menu level and page
//		    if (dashboard_menu_indent_level == 1 && main_dashboardPageIndex == 1) {
//		        onboardLed_blue_on();
//
//		        /* RELAXED DLC CHECK:
//		         * Support short CAN frames (DLC < 8) by checking only if the base header (4 + offset) exists.
//		         * This prevents rejecting valid UDS responses that end exactly on the data byte.
//		         */
//		        if (rx_msg_header.DLC >= (4 + (int)p->replyOffset)) {
//
//		            uint8_t bytesToRead = (p->replyLen > 4) ? 4 : p->replyLen;
//		            uint32_t tmpVal = 0;
//
//		            // Loop through the bytes defined in replyLen
//		            for (int i = 0; i < (int)bytesToRead; i++) {
//
//		                /* SAFE INDEX CALCULATION:
//		                 * Use 'int' to allow negative offsets (e.g., -1 to read byte 3).
//		                 * This fixes the issue where some ECUs send shorter UDS headers.
//		                 */
//		                int targetIdx = 4 + (int)p->replyOffset + i;
//
//		                /* OUT-OF-BOUNDS PROTECTION:
//		                 * Only read if the calculated index is within the physically received CAN frame.
//		                 * This prevents reading "garbage" or zeros from RAM beyond the actual DLC.
//		                 */
//		                if (targetIdx >= 0 && targetIdx < (int)rx_msg_header.DLC) {
//		                    tmpVal |= ((uint32_t)rx_msg_data[targetIdx]) << (8 * (bytesToRead - 1 - i));
//		                }
//		            }
//
//		            // 2. APPLY OFFSETS AND SCALING
//		            // Add raw value offset from the table
//		            tmpVal += p->replyValOffset;
//
//		            // Calculate final float value using scale and scale-offset
//		            float tmpVal2 = (tmpVal * p->replyScale) + p->replyScaleOffset;
//
//		            // Logic for single vs double parameter display
//		            if (uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[currentParamElementSelection] ==
//		                uds_params_array[function_is_diesel_enabled][dashboardPageIndex].udsParamId[!currentParamElementSelection]) {
//		                currentParamElementSelection = 0;
//		            }
//
//		            // Store the acquired parameter and send to the display slave
//		            dashboardParamCouple[currentParamElementSelection] = tmpVal2;
//		            sendDashboardPageToSlaveBaccable();
//		        }
//		    }
//		}


		if((seatbeltAlarmDisabled==0x11) || (seatbeltAlarmDisabled==0x21)){ //if write param was sent (seatbelt disabling or enabling in progress)
			if(rx_msg_header.ExtId==0x18DAF160){ //if received message comes from IPC
				if (rx_msg_header.DLC>=4){ //if at least 4 bytes
					if(rx_msg_data[1]==0x6F){ //if write param reply successful
						if((rx_msg_data[2]==0x55) && (rx_msg_data[3]==0xA0)){ //if param wrote was 55A0 (en/dis seatbelt alam)
							if(seatbeltAlarmDisabled==0x11){ // if seatbelt disabling
								seatbeltAlarmDisabled=1; //seatbelt disabled
							}
							if(seatbeltAlarmDisabled==0x21){ // if seatbelt enabling
								seatbeltAlarmDisabled=0; //seatbelt enabled
							}
						}
					}
				}
			}
		}


		if((seatbeltAlarmDisabled==0x10) || (seatbeltAlarmDisabled==0x20)){ //if diag session was sent (seatbelt disabling or enabling is in progress)
			if(rx_msg_header.ExtId==0x18DAF160){ //if received message comes from IPC
				if (rx_msg_header.DLC>=2){ //if at least 2 bytes
					if(rx_msg_data[1]==0x50){ //if diag session reply successful
						// Send enable/disable seatbelt alarm message
						uds_parameter_request_msg_header.ExtId=0x18DA60F1;
						uds_parameter_request_msg_header.DLC=6;
						uds_parameter_request_msg_data[0]=0x05;
						uds_parameter_request_msg_data[1]=0x2F;
						uds_parameter_request_msg_data[2]=0x55;
						uds_parameter_request_msg_data[3]=0xA0;
						uds_parameter_request_msg_data[4]=0x03;

						if(seatbeltAlarmDisabled==0x10){ // if seatbelt disabling
							uds_parameter_request_msg_data[5]=0x00; //set byte to disable alarm
						}
						if(seatbeltAlarmDisabled==0x20){ // if seatbelt enabling
							uds_parameter_request_msg_data[5]=0x01; //send msg to enable alarm
						}
						// send message
						can_tx(&uds_parameter_request_msg_header, uds_parameter_request_msg_data); //transmit the diag session request

						seatbeltAlarmDisabled++; //record that operation was executed
						seatbeltAlarmStatusRequestTime=currentTime;
						last_sent_uds_parameter_request_Time=currentTime;
					}
				}
			}
		}



		if(function_route_msg_enabled==1){
			if (rx_msg_header.ExtId==0x18DABAF1){ //if route request and dashboard menu not shown to avoid conflicts
				if (rx_msg_header.DLC>=7){
					routeStdIdMsg=!(rx_msg_data[2]>>4); //standard or extended msgID route request
					routeOffset=(rx_msg_data[2] & 0x0F); //offset from which start to copy
					routeMsgData[2]=rx_msg_data[2]; //copy in the response

					routeMsgId=	((uint32_t)rx_msg_data[3] << 24) |  // MSB
								((uint32_t)rx_msg_data[4] << 16) |
								((uint32_t)rx_msg_data[5] << 8)  |
								((uint32_t)rx_msg_data[6]);       	 // LSB



					onboardLed_blue_on();
				}
			}

			if(baccableDashboardMenuVisible) routeStdIdMsg=0xff; //disables the route request, to avoid conflicts with show params functionality

			if(routeStdIdMsg==0){ //if we have to do it (ext id route request)
				if(rx_msg_header.ExtId==routeMsgId){ //received msg to route
					routeStdIdMsg=0xFF; //set this to disable the request. only one message is routed to avoid bus flood
					if(routeOffset<rx_msg_header.DLC){ //send only if offset is correct
						uint8_t sizeToCopy=5; //
						if((rx_msg_header.DLC - routeOffset )<sizeToCopy) sizeToCopy=rx_msg_header.DLC - routeOffset;
						memcpy(&routeMsgData[3],&rx_msg_data[routeOffset],sizeToCopy);
						if(sizeToCopy<5) memset(&routeMsgData[3+sizeToCopy],0x00, 5-sizeToCopy);

						//send it
						can_tx(&routeMsgHeader, routeMsgData);
						onboardLed_blue_on();
					}

				}
			}
		}
	#endif //end define C1

	#if defined(C2baccable)
		if (rx_msg_header.ExtId==0x18DAF128 && DynoStateMachine!=0xff ){ //if message from ABS ECU and Dyno state machine is in progress
			if (DynoStateMachine==0 && rx_msg_header.DLC>=3){ //we received a reply to diagnostic session request msg
				if(rx_msg_data[0]==0x06 && rx_msg_data[1]==0x50 && rx_msg_data[2]==0x03){ //if request was successful
					DynoStateMachine++; //send dyno sts msg
				}
			}
			if (DynoStateMachine==1 && rx_msg_header.DLC>=5){ //we received a reply to dyno status msg
				if(rx_msg_data[0]==0x05 && rx_msg_data[1]==0x62 && rx_msg_data[2]==0x30 && rx_msg_data[3]==0x02){ //if request was successful
					DynoStateMachine++; //send dyno disable
					if(rx_msg_data[4]==0x00){ //if it is disabled, we shall enable it
						DynoModeEnabled=0;//refresh current status
						DynoStateMachine++;//send dyno enable
					}else{ //it is enabled, we shall disable it
						DynoModeEnabled=1;//refresh current status
					}
				}
			}
			if (DynoStateMachine==2 && rx_msg_header.DLC>=4){ //we received a reply to dyno disable msg
				if(rx_msg_data[0]==0x03 && rx_msg_data[1]==0x6E && rx_msg_data[2]==0x30 && rx_msg_data[3]==0x02){ //if request was successful
					DynoModeEnabled=0;//success change complete
					DynoStateMachine=0xff; //disable state machine

					//send message to master to inform about the status of Dyno
					uint8_t tmpArr2[2]={C1BusID,C1cmdDynoNotActive};
					if(DynoModeEnabled) tmpArr2[1]=C1cmdDynoActive;
					addToUARTSendQueue(tmpArr2, 2);

					onboardLed_blue_on();
				}
			}
			if (DynoStateMachine==3 && rx_msg_header.DLC>=4){ //we received a reply to dyno enable msg
				if(rx_msg_data[0]==0x03 && rx_msg_data[1]==0x6E && rx_msg_data[2]==0x30 && rx_msg_data[3]==0x02){ //if request was successful
					DynoModeEnabled=1;//success change complete

					DynoStateMachine=0xff; //disable state machine

					//send message to master to inform about the status of Dyno
					uint8_t tmpArr2[2]={C1BusID,C1cmdDynoNotActive};
					if(DynoModeEnabled) tmpArr2[1]=C1cmdDynoActive;
					addToUARTSendQueue(tmpArr2, 2);

					onboardLed_blue_on();
				}
			}

			if (DynoStateMachine!=0xff && rx_msg_header.DLC>=3){ //in any case
				if( rx_msg_data[1]==0x7F ){ //if request refused, abort all
					DynoStateMachine=0xff; //disable state machine

					//send message to master to inform about the status of Dyno
					uint8_t tmpArr2[2]={C1BusID,C1cmdDynoNotActive};
					if(DynoModeEnabled) tmpArr2[1]=C1cmdDynoActive;
					addToUARTSendQueue(tmpArr2, 2);

					onboardLed_blue_on();

				}
			}
			if(DynoStateMachine!=0xff){ //if we are running, send next message
				DYNO_msg_header.DLC=DYNO_msg_data[DynoStateMachine][0]+1;
				can_tx(&DYNO_msg_header, DYNO_msg_data[DynoStateMachine]); //add to the transmission queue
				onboardLed_blue_on();
				DynoStateMachineLastUpdateTime=currentTime;//save last time it was updated
			}
		}
	#endif  //end define C2

}
