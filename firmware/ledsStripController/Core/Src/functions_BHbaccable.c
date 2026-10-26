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
//		#define reverseAutoMuteEnabled 1
//		if(reverseAutoMuteEnabled){
//
//			// 1. STATE: REVERSE GEAR (0x0E) - Activate mute or log manual bypass
//			if(currentGear == 0x0E) {
//				exitReverseAudioTime = 0; // Reset timer while in Reverse
//
//				if(audioSystemReverseMuted == 0) {
//					if(audioSystemMuted == 0) {
//						// Music is playing -> copy from pure radio buffer and inject Mute button
//						memcpy(centerConsoleTxMsgData, (uint8_t*)centerConsoleRxMsgData, 6);
//						centerConsoleTxMsgData[2] = 0xE0; // Press Mute
//						can_tx(&centerConsoleTxMsgHeader, centerConsoleTxMsgData);
//
//						audioSystemReverseMuted = 1; // Locked by our automation (Safe rygiel)
//					} else {
//						audioSystemReverseMuted = 2; // User already muted the radio, bypass automation permanently
//					}
//				}
//			}
//			// 2. STATE: PARKING (0x0D) OR NEUTRAL (0x00) - Immediate restoration (Only for state 1)
//			else if(currentGear == 0x0D || currentGear == 0x00) {
//				exitReverseAudioTime = 0;
//
//				if(audioSystemReverseMuted == 1) { // Process ONLY if muted by us
//					if(audioSystemMuted == 1) { // Double check if radio is still muted
//						memcpy(centerConsoleTxMsgData, (uint8_t*)centerConsoleRxMsgData, 6);
//						centerConsoleTxMsgData[2] = 0xE0; // Press Mute again to toggle PLAY
//						can_tx(&centerConsoleTxMsgHeader, centerConsoleTxMsgData);
//					}
//				}
//				audioSystemReverseMuted = 0; // Safe reset to idle for both state 1 and 2
//			}
//			// 3. STATE: DRIVE (D) OR ANY OTHER GEAR - Safe delay only for automation
//			else {
//				// [NEW SAFE FIX]: If user manually unmuted on R, clear the state instantly on D entry (no delay needed)
//				if(audioSystemReverseMuted == 1 && audioSystemMuted == 0) {
//					audioSystemReverseMuted = 0;
//					exitReverseAudioTime = 0;
//				}
//
//				// If bypass state 2 is active, clear it instantly when moving forward (no delay needed)
//				if(audioSystemReverseMuted == 2) {
//					audioSystemReverseMuted = 0;
//					exitReverseAudioTime = 0;
//				}
//
//				// If muted by us (state 1) and timer is not armed yet, start countdown
//				if(audioSystemReverseMuted == 1 && exitReverseAudioTime == 0) {
//					exitReverseAudioTime = currentTime;
//				}
//
//				// Trigger return only after 3 seconds of constant driving forward
//				if(exitReverseAudioTime != 0 && (currentTime - exitReverseAudioTime > 3000)) {
//					if(audioSystemReverseMuted == 1) {
//						if(audioSystemMuted == 1) { // Restore only if still muted
//							memcpy(centerConsoleTxMsgData, (uint8_t*)centerConsoleRxMsgData, 6);
//							centerConsoleTxMsgData[2] = 0xE0; // Toggle back to PLAY
//							can_tx(&centerConsoleTxMsgHeader, centerConsoleTxMsgData);
//						}
//					}
//					audioSystemReverseMuted = 0; // Reset state machine to idle
//					exitReverseAudioTime = 0; // Reset timer after execution
//				}
//			}
//		}


//		#define reverseAutoMuteEnabled 1
//		if(reverseAutoMuteEnabled){
//
//			// REVERSE GEAR (0x0E) - Activate mute or log manual bypass
//			if(currentGear == 0x0E) {
//				// ==========================================
//				// PHASE 1: REVERSE GEAR -> AUTO-MUTE LOOPS
//				//(the loop is necessary because after ENGINE STARTS mute button us ignored for several seconds
//				// ==========================================
//				exitReverseAudioTime = 0; // Reset return timer while actively in Reverse
//
//				// Throttle check interval to exactly 500ms to allow ETM boot/processing time
//				if(currentTime - lastMuteSpamTime > 500) {
//
//					// [TRUTH CHECK]: Inject only if radio is playing (0) AND automation is idle (0)
//					// If the system enters State 2 (Manual Bypass), this block is completely locked down.
//					if(audioSystemMuted == 0 && audioSystemMutedByUs == 0) {
//						memcpy(centerConsoleTxMsgData, (uint8_t*)centerConsoleRxMsgData, 6);
//						centerConsoleTxMsgData[2] = 0xE0; // Press Mute button
//						can_tx(&centerConsoleTxMsgHeader, centerConsoleTxMsgData);
//
//						// Save transmission time ONLY when the frame is actually fired
//						lastMuteSpamTime = currentTime;
//					} else {
//						lastMuteSpamTime = currentTime;
//					}
//				}
//
//				// [CONFIRMED SUCCESS]: Acknowledge automation success (State 1) ONLY if radio reports muted (1)
//				// and our module actively initiated the fight by firing a frame (lastMuteSpamTime != 0).
//				if(audioSystemMuted == 1 && lastMuteSpamTime != 0 && audioSystemMutedByUs == 0) {
//					audioSystemMutedByUs = 1;
//				}
//
//				// [MANUAL UNMUTE BYPASS]: If automated mute was active (State 1) but user manually unmuted (0)
//				// via the volume knob while still in R, we shift into State 2 (Bypass) to surrender control.
//				if(audioSystemMutedByUs == 1 && audioSystemMuted == 0) {
//					audioSystemMutedByUs = 2; // Hard bypass lock until gear shift occurs
//				}
//			}
//
//			else {
//				// ==========================================
//				// PHASE 2: OTHER GEARS -> DELAYED AUDIO RESTORATION
//				// ==========================================
//
//				// Arm the 3-second restoration timer ONLY if audio was successfully muted by our automation (State 1)
//				if(audioSystemMutedByUs == 1 && exitReverseAudioTime == 0) {
//					exitReverseAudioTime = currentTime;
//				}
//
//				// Trigger audio restoration after 3 seconds (3000ms) of constant forward driving or parking/neutral
//				if(exitReverseAudioTime != 0 && (currentTime - exitReverseAudioTime > 3000)) {
//
//					if(audioSystemMutedByUs == 1) {
//						if(audioSystemMuted == 1) {
//							memcpy(centerConsoleTxMsgData, (uint8_t*)centerConsoleRxMsgData, 6);
//							centerConsoleTxMsgData[2] = 0xE0; // Press Mute again to toggle PLAY
//							can_tx(&centerConsoleTxMsgHeader, centerConsoleTxMsgData);
//						}
//					}
//
//					// Pure system reset - clear all states and clocks for the next session
//					audioSystemMutedByUs = 0;
//					lastMuteSpamTime = 0;
//					exitReverseAudioTime = 0;
//				}
//
//				// [BYPASS CLEANUP]: If leaving Reverse in manual bypass (State 2),
//				// silently clear the flags without transmitting any frames to avoid breaking user choice.
//				if(audioSystemMutedByUs == 2) {
//					audioSystemMutedByUs = 0;
//					lastMuteSpamTime = 0;
//					exitReverseAudioTime = 0;
//				}
//			}
//		}
//			//@netzmark MUTE_ON_REVERSE code - end

		// --- REVERSE AUDIO MUTE SYSTEM ---
		#define reverseAutoMuteEnabled 1
		if (reverseAutoMuteEnabled) {

		    // =========================================================================
		    // PHASE 1: REVERSE GEAR (0x0E) -> EVENT-DRIVEN AUTOMATION & FEEDBACK LOOP
		    // =========================================================================
		    if (currentGear == 0x0E) {
		        exitReverseAudioTime = 0; // Reset return timer while actively in Reverse

		        // 1. INITIALIZATION: Detect initial state or intent upon entering Reverse
		        if (audioSystemMutedByUs == 0) {
		            if (audioSystemMuted == 1) {
		                // User intentionally muted the radio BEFORE shifting to R -> Hard Bypass
		                audioSystemMutedByUs = 3;
		            } else {
		                // Radio is playing -> Transition to ATTEMPT state and fire immediately
		                audioSystemMutedByUs = 1;
		                memcpy(centerConsoleTxMsgData, (uint8_t*)centerConsoleRxMsgData, 6);
		                centerConsoleTxMsgData[2] = 0xE0; // Press Mute button
		                can_tx(&centerConsoleTxMsgHeader, centerConsoleTxMsgData);
		                lastMuteSpamTime = currentTime;
		            }
		        }

		        // 2. THROTTLE CHECK & RETRY LOOP: Keep fighting during ETM boot time (State 1 only)
		        if (audioSystemMutedByUs == 1 && audioSystemMuted == 0) {
		            if (currentTime - lastMuteSpamTime > 500) {
		                memcpy(centerConsoleTxMsgData, (uint8_t*)centerConsoleRxMsgData, 6);
		                centerConsoleTxMsgData[2] = 0xE0; // Repress Mute button
		                can_tx(&centerConsoleTxMsgHeader, centerConsoleTxMsgData);
		                lastMuteSpamTime = currentTime;
		            }
		        }

		        // 3. CONFIRMED SUCCESS: Lock automation only when radio reports muted (0x30)
		        if (audioSystemMutedByUs == 1 && audioSystemMuted == 1) {
		            audioSystemMutedByUs = 2;
		        }

		        // 4. MANUAL UNMUTE BYPASS: If automated mute was successful (State 2) but user
		        // turns the volume knob or presses mute on R, shift to State 3 to surrender control.
		        if (audioSystemMutedByUs == 2 && audioSystemMuted == 0) {
		            audioSystemMutedByUs = 3;
		        }
		    }

		    // =========================================================================
		    // PHASE 2: OTHER GEARS -> IMMEDIATE (P/N) OR DELAYED (D) AUDIO RESTORATION
		    // =========================================================================
		    else {
		        // SCENARIO A: IMMEDIATE RESTORATION FOR PARKING (0x0D) OR NEUTRAL (0x00)
		        if (currentGear == 0x0D || currentGear == 0x00) {
		            exitReverseAudioTime = 0;

		            if (audioSystemMutedByUs == 2) { // Process ONLY if muted by our automation
		                if (audioSystemMuted == 1) { // Double check if radio is still muted
		                    memcpy(centerConsoleTxMsgData, (uint8_t*)centerConsoleRxMsgData, 6);
		                    centerConsoleTxMsgData[2] = 0xE0; // Toggle back to PLAY immediately
		                    can_tx(&centerConsoleTxMsgHeader, centerConsoleTxMsgData);
		                }
		            }
		            // Pure system reset to Idle
		            audioSystemMutedByUs = 0;
		            lastMuteSpamTime = 0;
		        }

		        // SCENARIO B: DELAYED RESTORATION FOR DRIVE (D) OR ANY OTHER FORWARD GEAR
		        else {
		            // Arm the 3-second restoration timer only for successful automation (State 2)
		            if (audioSystemMutedByUs == 2 && exitReverseAudioTime == 0) {
		                exitReverseAudioTime = currentTime;
		            }

		            // Trigger return after 3 seconds of constant driving forward
		            if (exitReverseAudioTime != 0 && (currentTime - exitReverseAudioTime > 3000)) {
		                if (audioSystemMutedByUs == 2) {
		                    if (audioSystemMuted == 1) { // Restore only if still muted
		                        memcpy(centerConsoleTxMsgData, (uint8_t*)centerConsoleRxMsgData, 6);
		                        centerConsoleTxMsgData[2] = 0xE0; // Toggle back to PLAY
		                        can_tx(&centerConsoleTxMsgHeader, centerConsoleTxMsgData);
		                    }
		                }
		                // Pure system reset to Idle after execution
		                audioSystemMutedByUs = 0;
		                lastMuteSpamTime = 0;
		                exitReverseAudioTime = 0;
		            }

		            // SCENARIO C: CLEANUP FOR BYPASS OR UNFINISHED ATTEMPTS ON FORWARD GEAR
		            // Silently clear states without transmitting to respect manual choices or quick shifts
		            if (audioSystemMutedByUs == 1 || audioSystemMutedByUs == 3) {
		                audioSystemMutedByUs = 0;
		                lastMuteSpamTime = 0;
		                exitReverseAudioTime = 0;
		            }
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
