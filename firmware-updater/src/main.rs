use std::collections::VecDeque;
use std::env;
use std::fs;
use std::iter;
use std::process::exit;
use std::thread::sleep;
use std::time::Duration;
use std::usize;

use crc::CRC_32_BZIP2;
use crc::Crc;
use indicatif::{ProgressBar,ProgressStyle};
use serialport::{FlowControl, SerialPort};

const PACKET_LENGTH_BYTES:u8   = 1;
const PACKET_DATA_BYTES:u8     = 19;
const PACKET_CRC_BYTES:u8      = 4;
const PACKET_SIZE:u8         = PACKET_LENGTH_BYTES + PACKET_DATA_BYTES + PACKET_CRC_BYTES;

const PACKET_RET_BYTE0:u8     = 0x07;
const PACKET_ACK_BYTE0:u8     = 0x06;
const PACKET_PADDING_BYTE:u8     = 0xFF;

const SERIAL_PORT:&str = "COM4";
const BAUD_RATE:u32 = 115200;
const CRC:Crc<u32> = Crc::<u32>::new(&CRC_32_BZIP2);

const FU_PACKET_SYN_OBSERVED_BYTE0:u8 = 0x01;
const FU_PACKET_UP_REQ_BYTE0:u8 =0x1A;
const FU_PACKET_UP_RESP_BYTE0:u8 = 0x1B;
const FU_PACKET_FW_LENGTH_REQ_BYTE0:u8 = 0x0F;
const FU_PACKET_READY_FOR_FW_BYTE0:u8 = 0x02;
const FU_PACKET_UPDATE_SUCCESS_BYTE0:u8 = 0x04;

const FU_SYNC_SEQ:[u8;4] = [0xDE,0xAD,0xBA,0xBE];

const BOOTLOADER_SIZE:usize = 0x4000;

#[derive(Clone)]
struct Packet{
    len:u8,
    data:Vec<u8>,
    crc:u32,
}

impl Packet{

    pub fn new(len:u8,data:&[u8])->Packet{
        let mut pkt = Self{
            len,
            data:data.into(),
            crc:0
        };

        if pkt.data.len() < PACKET_DATA_BYTES as usize {
            let bytes_to_pad = PACKET_DATA_BYTES as usize - pkt.data.len();
            let padding:Vec<u8> = Vec::from_iter(iter::repeat(PACKET_PADDING_BYTE).take(bytes_to_pad));
            pkt.data.extend(&padding);
        }

        pkt.compute_crc(); 
        
        return pkt;
    }

    pub fn compute_crc(&mut self){
        let mut payload:Vec<u8> = Vec::with_capacity((PACKET_LENGTH_BYTES + PACKET_DATA_BYTES) as usize);
        payload.push(self.len);
        payload.extend(&self.data);
        self.crc = CRC.checksum(&payload);
    }

    pub fn as_bytes(&self)->Vec<u8>{
        let mut pkt:Vec<u8> = Vec::with_capacity(PACKET_SIZE as usize);
        pkt.push(self.len);
        pkt.extend(&self.data);
        pkt.extend_from_slice(&self.crc.to_le_bytes());
        return pkt;
    }
    
    fn is_cntrl_packet(&self,byte:u8)->bool{
        if self.len != 1{ return false;}
        if self.data[0] != byte {return false;}
        if self.data.iter()
            .skip(1)
            .filter(|&item| *item != 0xFF )
            .count() != 0 {
            return false;
        }
        return true;
    }

    pub fn is_ack(&self)->bool{
        return self.is_cntrl_packet(PACKET_ACK_BYTE0);
    }

    pub fn is_ret(&self)->bool{
        return self.is_cntrl_packet(PACKET_RET_BYTE0);
    }

    pub fn replace(&mut self,value:&Self){
        self.len = value.len;
        for i in 0..self.data.len(){
            self.data[i] = value.data[i];
        }
        self.crc = value.crc;
    }
}

impl From<u8> for Packet {
    fn from(value: u8) -> Self {
        return Self::new(1, &[value]);
    }
}


fn main() {
    let fw_update_path = "./fw-image/firmware.bin";

    let fw_data = fs::read(fw_update_path).unwrap();
    let final_data:Vec<u8> = fw_data.into_iter().skip(BOOTLOADER_SIZE).collect();

    let progress = ProgressBar::new(final_data.len() as u64);
    progress.set_style(ProgressStyle::with_template("[{bar:40.cyan/blue}] {percent}%")
        .unwrap()
        .progress_chars("#>-"));
    
    let ports = serialport::available_ports().expect("No serial ports found!");
    ports.iter()
        .find(|item|{
            return item.port_name == SERIAL_PORT.to_owned();
        })
        .expect("Targeted serail port not found!");
    
    let mut uart = 
        serialport::new(SERIAL_PORT, BAUD_RATE)
        .parity(serialport::Parity::None)
        .flow_control(FlowControl::None)
        .stop_bits(serialport::StopBits::One)
        .open().unwrap();

    println!("[+] Device found on port {}",uart.name().unwrap());

    let mut packets_queue: VecDeque<Packet> = VecDeque::new();
    let mut prev_packet:Packet =  Packet::new(1,&[0xFF]);
    let mut result = sync_with_bootloader(&mut uart, &mut packets_queue, &mut prev_packet,10000);
    if !result {
        println!("[-] Sync Timeout!!");
        exit(1);
    }
    let mut reply = Packet::from(FU_PACKET_UP_REQ_BYTE0);
    send_packet(&mut uart, &reply, &mut prev_packet);
    println!("[+] Sending Update request.");
    result = wait_for_cntrl_packet(&mut uart,&mut packets_queue,&mut prev_packet, FU_PACKET_UP_RESP_BYTE0, 5000);
    if !result {
        println!("[-] Update Timeout!!");
        exit(1);
    }
    result = wait_for_cntrl_packet(&mut uart, &mut packets_queue,&mut prev_packet, FU_PACKET_FW_LENGTH_REQ_BYTE0, 5000);
    if !result {
        println!("[-] Update Timeout!!");
        exit(1);
    }
    println!("[+] Bootloader requested firmware length");
    reply = create_firmware_length_packet(final_data.len());
    send_packet(&mut uart, &reply, &mut prev_packet);
    println!("[+] Responding with {}Kib",final_data.len());
    result = wait_for_cntrl_packet(&mut uart,&mut packets_queue, &mut prev_packet, FU_PACKET_READY_FOR_FW_BYTE0, 5000);
    if !result {
        println!("[-] Update Timeout!!");
        exit(1);
    }
    let mut bytes_sent:usize = 0;
    println!("[+] Sending Firmware");
    while bytes_sent < final_data.len() {
        let increment = if final_data.len() - bytes_sent > PACKET_DATA_BYTES as usize {
            PACKET_DATA_BYTES as usize
        }else{
            final_data.len() - bytes_sent
        };

        let data_bytes = &final_data[bytes_sent..(bytes_sent + increment)];
        let data_pkt = Packet::new(increment as u8, data_bytes);
        send_packet(&mut uart, &data_pkt, &mut prev_packet);
        if bytes_sent + increment < final_data.len(){
            if !wait_for_cntrl_packet(&mut uart,&mut packets_queue,&mut prev_packet, FU_PACKET_READY_FOR_FW_BYTE0, 5000){
                println!("[-] Update Failure!!");
                exit(0);
            }
        }
        bytes_sent += increment;
        // println!("[+] Wrote {} bytes of firmware {:.2}%",bytes_sent,(bytes_sent as f32/final_data.len() as f32)*100.0);
        progress.set_position(bytes_sent as u64);
    }
    result = wait_for_cntrl_packet(&mut uart,&mut packets_queue,&mut prev_packet, FU_PACKET_UPDATE_SUCCESS_BYTE0, 5000);
    if !result {
        println!("[-] Bootloader didn't confirm update!!");
        exit(1);
    }
    println!("[+] Update Success.");
    exit(0);

}

fn poll_queue(uart:&mut Box<dyn SerialPort>,packets_queue:&mut VecDeque<Packet>,prev:&mut Packet){
    let ack:Packet = Packet::new(1, &[PACKET_ACK_BYTE0]);
    let ret:Packet = Packet::new(1, &[PACKET_RET_BYTE0]);
    let mut raw_data:[u8;PACKET_SIZE as usize] = [0x00;PACKET_SIZE as usize];
    while uart.bytes_to_read().unwrap() >= PACKET_SIZE as u32 {
        match uart.read_exact(raw_data.as_mut_slice()){
                Ok(()) => {},
                Err(e) => {
                    println!("[ERROR]: {}",e);
                    exit(1);
                }
        }

        // println!("raw_data={:?}",raw_data);

        let pkt = match parse_packet(&raw_data){
            Some(val) => val,
            None => {
                send_packet(uart, &ret,prev);
                continue;
            }
        };

        // Request Retransmit
        if pkt.is_ret() {
            // println!("Retransmitting last packet");
            write_packet(uart, prev);
            continue;
        }

        // Ack packet
        if pkt.is_ack(){
            // println!("Received Ack packet");
            continue;
        }

        // Save the packet and send ACK
        packets_queue.push_back(pkt);
        send_packet(uart, &ack, prev);
    }
}

fn create_firmware_length_packet(length:usize)->Packet{
    let mut reply_payload:[u8;4] = [0;4];
    let length_bytes = length.to_le_bytes();
    reply_payload[0] = length_bytes[0];
    reply_payload[1] = length_bytes[1];
    reply_payload[2] = length_bytes[2];
    reply_payload[3] = length_bytes[3];
    return Packet::new(reply_payload.len() as u8,&reply_payload);
}

fn wait_for_cntrl_packet(uart:&mut Box<dyn SerialPort>,
    packets_queue:&mut VecDeque<Packet>,
    prev:&mut Packet,
    value:u8,
    timeout:u32) -> bool  {
    let mut iterations:u32 = 0;
    while iterations < timeout {
        sleep(Duration::from_millis(100));
        poll_queue(uart, packets_queue, prev);
        if  packets_queue.len() > 0{
            let pkt = packets_queue.pop_front().unwrap();
            // println!("checking packt={:?}",pkt.as_bytes());
            if pkt.is_cntrl_packet(value){
                // println!("check true");
                return true;
            }
                // println!("check false");
        }
        iterations += 100;
    }
    return false;
}

fn sync_with_bootloader(uart:&mut Box<dyn SerialPort>,
    packets_queue:&mut VecDeque<Packet>,
    prev:&mut Packet,
    timeout:usize) -> bool  {
    println!("[+] Syncing with bootloader");
    let mut iterations:usize = 0;
    while iterations < timeout {
        match uart.write_all(&FU_SYNC_SEQ){
            Ok(_) => {},
            Err(e) => {
                println!("[ERROR]:{}",e);
                return false;
            }
        }
        // uart.flush().expect("[ERROR]: UART write failed!");
        sleep(Duration::from_millis(100));
        poll_queue(uart, packets_queue, prev);
        if  packets_queue.len() > 0{
            let pkt = packets_queue.pop_front().unwrap();
            if pkt.is_cntrl_packet(FU_PACKET_SYN_OBSERVED_BYTE0){
                return true;
            }
        }
        iterations += 10;
    }
    return false;
}

fn parse_packet(data:&[u8]) -> Option<Packet> {
    assert!(data.len() == 24);
    let pkt = Packet::new(data[0],&data[1..20]);
    let be_crc:[u8;4] = [data[20],data[21],data[22],data[23]];
    let recvd_crc:u32 = u32::from_le_bytes(be_crc);

    // Packet corrupted 
    if pkt.crc != recvd_crc {
        println!("CRC failed, computed {:#02x}, got {:#02x}",pkt.crc,recvd_crc);
        return None;
    }
    return Some(pkt);
}

fn send_packet(uart_guard:&mut Box<dyn SerialPort>,
    pkt:&Packet,
    prev:&mut Packet) -> bool{
    if write_packet(uart_guard, pkt){
        prev.replace(pkt);
        return true;
    }
    return false;
}


fn write_packet(uart:&mut Box<dyn SerialPort>,pkt:&Packet)->bool{
    // println!("Sending={:?}",pkt.as_bytes());
    match uart.write_all(&pkt.as_bytes()){
        Ok(_) => {},
        Err(e) => {
            println!("[ERROR]:{}",e);
        }
    }
    // uart.flush().expect("[ERROR]: UART write failed!");
    return true;
}
