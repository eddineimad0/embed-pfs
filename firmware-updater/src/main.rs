use std::env;
use std::iter;
use std::process::exit;
use std::time::Duration;
use std::usize;

use crc::CRC_32_BZIP2;
use crc::Crc;
use serialport::{DataBits, FlowControl, SerialPort};

const PACKET_LENGTH_BYTES:u8   = 1;
const PACKET_DATA_BYTES:u8     = 19;
const PACKET_CRC_BYTES:u8      = 4;
// const PACKET_CRC_POSITION:u8      = PACKET_LENGTH_BYTES + PACKET_DATA_BYTES;
const PACKET_SIZE:u8         = PACKET_LENGTH_BYTES + PACKET_DATA_BYTES + PACKET_CRC_BYTES;

const PACKET_RET_BYTE0:u8     = 0x15;
// const PACKET_SYN_BYTE0:u8     = 0x16;
const PACKET_ACK_BYTE0:u8     = 0x17;
const PACKET_PADDING_BYTE:u8     = 0xFF;

const SERIAL_PORT:&str = "COM4";
const BAUD_RATE:u32 = 115200;
const DEFAULT_TIMEOUT:Duration = Duration::from_millis(1000);
const CRC:Crc<u32> = Crc::<u32>::new(&CRC_32_BZIP2);

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
}

fn main() {
    env::set_var("RUST_BACKTRACE", "1");
    
    let ports = serialport::available_ports().expect("No serial ports found!");
    ports.iter()
        .find(|item|{
            return item.port_name == SERIAL_PORT.to_owned();
        })
        .expect("Targeted serail port not found!");
    
    let mut uart = serialport::new(SERIAL_PORT, BAUD_RATE)
        .data_bits(DataBits::Eight)
        .parity(serialport::Parity::None)
        .flow_control(FlowControl::None)
        .stop_bits(serialport::StopBits::One)
        .timeout(DEFAULT_TIMEOUT)
        .open().unwrap();

    println!("[+] Listening on {}",SERIAL_PORT);

    let ack:Packet = Packet::new(1, &[PACKET_ACK_BYTE0]);
    let ret:Packet = Packet::new(1, &[PACKET_RET_BYTE0]);
    let mut prev_packet:Packet = Packet::new(1,&[0xFF]);

    let mut packets:Vec<Packet> = Vec::new();
    let mut raw_data:[u8;PACKET_SIZE as usize] = [0x00;PACKET_SIZE as usize];

    loop {
        while uart.bytes_to_read().unwrap() >= PACKET_SIZE as u32 {
            match uart.read_exact(raw_data.as_mut_slice()){
                    Ok(()) => {},
                    Err(e) => {
                        println!("[ERROR]: {}",e);
                        exit(1);
                    }
            }

            println!("raw_data={:?}",raw_data);

            let pkt = Packet::new(raw_data[0],&raw_data[1..20]);
            let be_crc:[u8;4] = [raw_data[20],raw_data[21],raw_data[22],raw_data[23]];
            let recvd_crc:u32 = u32::from_le_bytes(be_crc);

            // Packet corrupted 
            if pkt.crc != recvd_crc {
                println!("CRC failed, computed {:#02x}, got {:#02x}",pkt.crc,recvd_crc);
                write_packet(&mut uart, &ret);
                prev_packet = ret.clone();
                continue;
            }

            // Request Retransmit
            if pkt.is_ret() {
                println!("Retransmitting last packet");
                println!("Last packet={:?}", prev_packet.as_bytes());
                write_packet(&mut uart, &prev_packet);
                continue;
            }

            // Ack packet
            if pkt.is_ack(){
                println!("Received Ack packet");
                continue;
            }
            // save the packet and send ACK
            packets.push(pkt);
            write_packet(&mut uart, &ack);
            prev_packet = ack.clone();
        }

    }
}

fn write_packet(uart:&mut Box<dyn SerialPort>,pkt:&Packet)->bool{
    match uart.write_all(&pkt.as_bytes()){
        Ok(_) => {},
        Err(e) => {
            println!("[ERROR]:{}",e);
        }
    }
    uart.flush().expect("[ERROR]: UART write failed!");
    return true;
}
