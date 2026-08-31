/*
 * functions_BHbaccable.c
 *
 *  Created on: May 2, 2025
 *      Author: GauchoHP
 */

#include "functions_BHbaccable.h"

#if defined(BHbaccable)

	void BHbaccableInitCheck(){
		//let's open the can bus because we may need data
		can_set_bitrate(CAN_BITRATE_125K);//set can speed to 125kpbs
		can_enable(); //enable can port

		//prepare msg to send:
		//total frame number is on byte 0 from bit 7 to 3
		telematic_display_info_msg_data[0]=(telematic_display_info_msg_data[0] & ~0xF8) | ((telematic_display_info_field_totalFrameNumber<<3) & 0xF8);
		//infoCode is on byte1 from bit 5 to 0 (0x12=phone connected, 0x13=phone disconnected, 0x15=call in progress, 0x17=call in wait, 0x18=call terminated, 0x11=clear display, ...)
		telematic_display_info_msg_data[1]=(telematic_display_info_msg_data[1] & ~0x3F) | ((telematic_display_info_field_infoCode) & 0x3F);
		//I don't use UTF chars, but ascii, so bytes 2,4,6 can be set to zero
		telematic_display_info_msg_data[2]=0;
		telematic_display_info_msg_data[4]=0;
		telematic_display_info_msg_data[6]=0;

		//load stored params
		leftParkMirrorHorizontalPos		=(uint8_t)readFromFlashBH(1);
		leftParkMirrorVerticalPos		=(uint8_t)readFromFlashBH(2);
		rightParkMirrorHorizontalPos	=(uint8_t)readFromFlashBH(3);
		rightParkMirrorVerticalPos		=(uint8_t)readFromFlashBH(4);
		storeCurrentParkMirrorPosition	=(uint8_t)readFromFlashBH(5);
		leftMirrorHorizontalOperativePos=(uint8_t)readFromFlashBH(6);
		leftMirrorVerticalOperativePos	=(uint8_t)readFromFlashBH(7);
		rightMirrorHorizontalOperativePos=(uint8_t)readFromFlashBH(8);
		rightMirrorVerticalOperativePos =(uint8_t)readFromFlashBH(9);

	}

	void BHperiodicCheck(){
		if(requestToSendOneFrame>0){ //if requested by a message received from master baccable
			//send one msg to write something on the dashboard each 50msec (one frame each 300msec)
			if (currentTime-lastSentTelematic_display_info_msg_Time>50){
				lastSentTelematic_display_info_msg_Time=currentTime;
				//prepare msg to send:
				//frame number is on byte 0 from bit 2 to 0 and byte1 from bit7 to 6
				telematic_display_info_msg_data[0]=(telematic_display_info_msg_data[0] & ~0x07) | ((telematic_display_info_field_frameNumber>>2) & 0x07);
				telematic_display_info_msg_data[1]=(telematic_display_info_msg_data[1] & ~0xC0) | ((telematic_display_info_field_frameNumber<<6) & 0xC0);

				//UTF text 1 is on byte 2 and byte 3
				telematic_display_info_msg_data[3]=dashboardPageStringArray[paramsStringCharIndex];
				paramsStringCharIndex++; //prepare to send next char
				//UTF text 2 is on byte 4 (set to zero ) and byte 5
				telematic_display_info_msg_data[5]=dashboardPageStringArray[paramsStringCharIndex];
				paramsStringCharIndex++; //prepare to send next char
				//UTF text 3 is on byte 6 (set to zero) and byte 7
				telematic_display_info_msg_data[7]=dashboardPageStringArray[paramsStringCharIndex];
				paramsStringCharIndex++; //prepare to send next char
				//send it
				can_tx(&telematic_display_info_msg_header, telematic_display_info_msg_data); //transmit the packet

				telematic_display_info_field_frameNumber++; //prepare for next frame to send
				if( paramsStringCharIndex>=DASHBOARD_MESSAGE_MAX_LENGTH) { //if we sent the entire string
					paramsStringCharIndex=0; //prepare to send first char of the string
					telematic_display_info_field_frameNumber=0; //prepare to send first frame
					requestToSendOneFrame -= 1;
					onboardLed_blue_on();
				}
			}
		}

		//--- @Gaucho original code replaced with @netzmark code below
//		if(function_park_mirror){ //if function parkmirror is enabled
//
//			if(currentGear==0x0E){ //if reverse gear is selected
//				if(!restoreOperativeMirrorsPosition){ //if we are not returning to operative position
//					switch(turnIndicator){
//						case 0x02: //left arrow inserted
//							if(!leftParkMirrorPositionRequired && !rightParkMirrorPositionRequired) storeOperativeMirrorPosition=1;//store current mirror position, if mirror was not previously lowered
//							leftParkMirrorPositionRequired=1; //Enable sending command to move mirror
//							break;
//						case 0x01: //right arrow inserted
//							if(!leftParkMirrorPositionRequired && !rightParkMirrorPositionRequired) storeOperativeMirrorPosition=1;//store current mirror position, if mirror was not previously lowered
//							rightParkMirrorPositionRequired=1; //Enable sending command to move mirror
//							break;
//						default:
//					}
//				}
//			}else{
//				if(leftParkMirrorPositionRequired || rightParkMirrorPositionRequired){ //if mirrors are potentially not in operative position,
//					restoreOperativeMirrorsPosition=1; //request to restore mirrors to their original position
//					restoreOperativeMirrorsPositionRequestTime=currentTime;
//				}
//				leftParkMirrorPositionRequired=0; //stop sending message to set Park position for mirrors
//				rightParkMirrorPositionRequired=0;//stop sending message to set Park position for mirrors
//			}
//
//
//			//Prepare msg to send: set Operative position of the mirrors
//			parkMirrorMsgData[0]= leftMirrorHorizontalOperativePos;
//			parkMirrorMsgData[1]= leftMirrorVerticalOperativePos;
//			parkMirrorMsgData[2]= rightMirrorHorizontalOperativePos;
//			parkMirrorMsgData[3]= rightMirrorVerticalOperativePos;
//
//			//Prepare msg to send: if required, set park position of the mirrors
//			if(leftParkMirrorPositionRequired){
//				parkMirrorMsgData[0]= leftParkMirrorHorizontalPos;
//				parkMirrorMsgData[1]= leftParkMirrorVerticalPos;
//			}
//			if(rightParkMirrorPositionRequired){
//				parkMirrorMsgData[2]= rightParkMirrorHorizontalPos;
//				parkMirrorMsgData[3]= rightParkMirrorVerticalPos;
//			}
//
//
//			if(leftParkMirrorPositionRequired || rightParkMirrorPositionRequired || restoreOperativeMirrorsPosition){ //if required
//				if(!storeOperativeMirrorPosition){ //if operative position was stored
//					if(currentTime-lastParkMirrorMsgTime>900){ //each 1000msec send a packet
//						can_tx(&parkMirrorMsgHeader, parkMirrorMsgData); //send msg
//						lastParkMirrorMsgTime=currentTime;
//					}
//				}
//				if(restoreOperativeMirrorsPosition){
//					if(currentTime-restoreOperativeMirrorsPositionRequestTime>15000){ //after 15 seconds
//						restoreOperativeMirrorsPosition=0;
//					}
//				}
//			}
//		}

		//--- @netzmark park mirror returning delay added - begin//
		if(function_park_mirror){ //if function parkmirror is enabled

			if(currentGear==0x0E){ //if reverse gear is selected 0x0E (masked 7E in fact)
				exitReverseTime = 0; //Reset hysteresis timer while in Reverse

				if(!restoreOperativeMirrorsPosition){ //if we are not returning to operative position
					switch(turnIndicator){
						case 0x02: //left arrow inserted
							if(!leftParkMirrorPositionRequired && !rightParkMirrorPositionRequired) storeOperativeMirrorPosition=1;//store current mirror position, if mirror was not previously lowered
							leftParkMirrorPositionRequired=1; //Enable sending command to move mirror
							break;
						case 0x01: //right arrow inserted
							if(!leftParkMirrorPositionRequired && !rightParkMirrorPositionRequired) storeOperativeMirrorPosition=1;//store current mirror position, if mirror was not previously lowered
							rightParkMirrorPositionRequired=1; //Enable sending command to move mirror
							break;
						default:
					}
				}
			}else if(currentGear == 0x0D || currentGear == 0x00){	//if Parking or Neutral (0x00) is selected we raise the mirrors immediately
																	//protection against leaving the mirrors lowered in case of ignition OFF within added delay time
																	//tested and working reliably on automatic gear box only
																 		//maybe it is not good for manual gear box where maybe N is always set during transition from R>D
																		//in this case remove this else if function
				exitReverseTime = 0; //Reset timer for P/N

				if(leftParkMirrorPositionRequired || rightParkMirrorPositionRequired){ //if mirrors are potentially not in operative position,
					restoreOperativeMirrorsPosition=1; //request to restore mirrors to their original position
					restoreOperativeMirrorsPositionRequestTime=currentTime;
				}
				leftParkMirrorPositionRequired=0; //stop sending message to set Park position for mirrors
				rightParkMirrorPositionRequired=0;//stop sending message to set Park position for mirrors
			}else{ //if Drive (D) or any other gear is selected
				//If mirrors are lowered and timer is not armed yet, capture the exit time
				if((leftParkMirrorPositionRequired || rightParkMirrorPositionRequired) && exitReverseTime == 0){
					exitReverseTime = currentTime; //Capture the moment we switched to D
				}

				//Trigger return only after 10 seconds of constant driving forward
				if(exitReverseTime != 0 && (currentTime - exitReverseTime > 10000)){ //HERE IT IS WHAT WE WANTED TO ADD - 10 seconds hysteresis delay
					if(leftParkMirrorPositionRequired || rightParkMirrorPositionRequired){ //if mirrors are potentially not in operative position,
						restoreOperativeMirrorsPosition=1; //request to restore mirrors to their original position
						restoreOperativeMirrorsPositionRequestTime=currentTime;
					}
					leftParkMirrorPositionRequired=0; //stop sending message to set Park position for mirrors
					rightParkMirrorPositionRequired=0;//stop sending message to set Park position for mirrors
					exitReverseTime = 0; //Reset timer after execution
				}
			}

			//Prepare msg to send: set Operative position of the mirrors
			parkMirrorMsgData[0]= leftMirrorHorizontalOperativePos;
			parkMirrorMsgData[1]= leftMirrorVerticalOperativePos;
			parkMirrorMsgData[2]= rightMirrorHorizontalOperativePos;
			parkMirrorMsgData[3]= rightMirrorVerticalOperativePos;

			//Prepare msg to send: if required, set park position of the mirrors
			if(leftParkMirrorPositionRequired){
				parkMirrorMsgData[0]= leftParkMirrorHorizontalPos;
				parkMirrorMsgData[1]= leftParkMirrorVerticalPos;
			}
			if(rightParkMirrorPositionRequired){
				parkMirrorMsgData[2]= rightParkMirrorHorizontalPos;
				parkMirrorMsgData[3]= rightParkMirrorVerticalPos;
			}


			if(leftParkMirrorPositionRequired || rightParkMirrorPositionRequired || restoreOperativeMirrorsPosition){ //if required
				if(!storeOperativeMirrorPosition){ //if operative position was stored
					if(currentTime-lastParkMirrorMsgTime>900){ //each 1000msec send a packet
						can_tx(&parkMirrorMsgHeader, parkMirrorMsgData); //send msg
						lastParkMirrorMsgTime=currentTime;
					}
				}
				if(restoreOperativeMirrorsPosition){
					if(currentTime-restoreOperativeMirrorsPositionRequestTime>15000){ //after 15 seconds
						restoreOperativeMirrorsPosition=0;
					}
				}
			}
		}
		//--- @netzmark park mirror returning delay added - end//


		// --- REVERSE AUDIO MUTE SYSTEM ---
		#define reverseAutoMuteEnabled 1
		if(reverseAutoMuteEnabled){

			// 1. STATE: REVERSE GEAR (0x0E) - Activate mute or log manual bypass
			if(currentGear == 0x0E) {
				exitReverseAudioTime = 0; // Reset timer while in Reverse

				if(audioSystemReverseMuted == 0) {
					if(audioSystemMuted == 0) {
						// Music is playing -> copy from pure radio buffer and inject Mute button
						memcpy(centerConsoleTxMsgData, (uint8_t*)centerConsoleRxMsgData, 6);
						centerConsoleTxMsgData[2] = 0xE0; // Press Mute
						can_tx(&centerConsoleTxMsgHeader, centerConsoleTxMsgData);

						audioSystemReverseMuted = 1; // Locked by our automation (Safe rygiel)
					} else {
						audioSystemReverseMuted = 2; // User already muted the radio, bypass automation permanently
					}
				}
			}
			// 2. STATE: PARKING (0x0D) OR NEUTRAL (0x00) - Immediate restoration (Only for state 1)
			else if(currentGear == 0x0D || currentGear == 0x00) {
				exitReverseAudioTime = 0;

				if(audioSystemReverseMuted == 1) { // Process ONLY if muted by us
					if(audioSystemMuted == 1) { // Double check if radio is still muted
						memcpy(centerConsoleTxMsgData, (uint8_t*)centerConsoleRxMsgData, 6);
						centerConsoleTxMsgData[2] = 0xE0; // Press Mute again to toggle PLAY
						can_tx(&centerConsoleTxMsgHeader, centerConsoleTxMsgData);
					}
				}
				audioSystemReverseMuted = 0; // Safe reset to idle for both state 1 and 2
			}
			// 3. STATE: DRIVE (D) OR ANY OTHER GEAR - Safe delay only for automation
			else {
				// [NEW SAFE FIX]: If user manually unmuted on R, clear the state instantly on D entry (no delay needed)
				if(audioSystemReverseMuted == 1 && audioSystemMuted == 0) {
					audioSystemReverseMuted = 0;
					exitReverseAudioTime = 0;
				}

				// If bypass state 2 is active, clear it instantly when moving forward (no delay needed)
				if(audioSystemReverseMuted == 2) {
					audioSystemReverseMuted = 0;
					exitReverseAudioTime = 0;
				}

				// If muted by us (state 1) and timer is not armed yet, start countdown
				if(audioSystemReverseMuted == 1 && exitReverseAudioTime == 0) {
					exitReverseAudioTime = currentTime;
				}

				// Trigger return only after 3 seconds of constant driving forward
				if(exitReverseAudioTime != 0 && (currentTime - exitReverseAudioTime > 3000)) {
					if(audioSystemReverseMuted == 1) {
						if(audioSystemMuted == 1) { // Restore only if still muted
							memcpy(centerConsoleTxMsgData, (uint8_t*)centerConsoleRxMsgData, 6);
							centerConsoleTxMsgData[2] = 0xE0; // Toggle back to PLAY
							can_tx(&centerConsoleTxMsgHeader, centerConsoleTxMsgData);
						}
					}
					audioSystemReverseMuted = 0; // Reset state machine to idle
					exitReverseAudioTime = 0; // Reset timer after execution
				}
			}
		}
		//@netzmark MUTE_ON_REVERSE code - end
	}

	uint8_t saveOnFlashBH(){
		//last page to store on flash is 0x0801 F800 (we can store 2 bytes each time)
		// and we shall erase entire page before write. one page size is FLASH_PAGE_SIZE (2048 bytes in st32F072)
		HAL_FLASH_Unlock(); //unlock flash

		//erase flash
		FLASH_EraseInitTypeDef eraseInitStruct;
		uint32_t pageError=0;
		eraseInitStruct.TypeErase= FLASH_TYPEERASE_PAGES;
		eraseInitStruct.PageAddress=LAST_PAGE_ADDRESS; //last page address begin
		eraseInitStruct.NbPages=1;
		if(HAL_FLASHEx_Erase(&eraseInitStruct,&pageError)!=HAL_OK){ //error during erase
			HAL_FLASH_Lock();
			onboardLed_red_blink(8);
			return 254; //error
		}

		//it seems that stm32F072 supports only writing 2byte words
		//write parameter
		uint8_t paramsNumber=9;
		uint16_t params[40] = {
				leftParkMirrorHorizontalPos,
				leftParkMirrorVerticalPos,
				rightParkMirrorHorizontalPos,
				rightParkMirrorVerticalPos,
				parkMirrorOperativePositionNotStored,
				leftMirrorHorizontalOperativePos,
				leftMirrorVerticalOperativePos,
				rightMirrorHorizontalOperativePos,
				rightMirrorVerticalOperativePos,

		};

		for (uint8_t i = 0; i < paramsNumber; i++) {
		    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, LAST_PAGE_ADDRESS + (i * 4), params[i]) != HAL_OK) {
		        HAL_FLASH_Lock();
		        onboardLed_red_blink(9);
		        return 255;
		    }
		}

		//lock the flash
		HAL_FLASH_Lock();
		return 0;

	}

	uint16_t readFromFlashBH(uint8_t paramId){
		if(paramId<1) return 0;
		uint16_t tmpParam=*(volatile uint16_t*)(LAST_PAGE_ADDRESS+((paramId-1)*4));
		return tmpParam;
	}

#endif
