// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use std::{
    env,
    fs::{read_to_string, write},
    os::windows::process::CommandExt,
    process::Command,
};

use serde::Serialize;
use serde_json::Value;

const CREATE_NO_WINDOW: u32 = 0x08000000;
const DETACHED_PROCESS: u32 = 0x00000008;

#[derive(Serialize)]
struct Request<T: Serialize> {
    uri: String,
    body: Option<T>,
}

#[tauri::command]
fn call(uri: &str, payload: Option<String>) -> Result<String, String> {
    let script_path = r#"D:\GUI\exec_cmd.vbs"#;
    let lock_file = r#"D:\GUI\.lock"#;
    let ipc = r#"D:\GUI\IPC.req.json"#;

    println!("{:#?}", payload);
    loop {
        let locked = if let Ok(metadata) = std::fs::metadata(lock_file) {
            metadata.is_file()
        } else {
            false
        };

        if !locked {
            break;
        }
    }

    std::fs::File::create(lock_file).ok();

    let req = if payload.is_some() {
        let payload: Value = serde_json::from_str(&payload.unwrap()).unwrap();
        Request {
            uri: uri.to_owned(),
            body: Some(payload),
        }
    } else {
        Request {
            uri: uri.to_owned(),
            body: None,
        }
    };

    if let Err(err) = write(ipc, serde_json::to_string(&req).unwrap()) {
        return Err(err.to_string());
    }

    let res = if Command::new("cscript")
        .creation_flags(DETACHED_PROCESS | CREATE_NO_WINDOW)
        .arg(script_path)
        .output()
        .is_ok()
    {
        if let Ok(content) =
            read_to_string(r#"D:\GUI\IPC.res.json"#)
        {
            Ok(content)
        } else {
            Err("No response".to_owned())
        }
    } else {
        Err("Failed to run command".to_owned())
    };

    std::fs::remove_file(lock_file).ok();

    return res;
}

fn main() {
    tauri::Builder::default()
        .invoke_handler(tauri::generate_handler![call])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
