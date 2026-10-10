#pragma once

namespace WebPages
{
    static constexpr char INDEX[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>CherryRF</title>

<style>
* { box-sizing:border-box; }

body {
    margin:0;
    background:#0c0c10;
    color:#eee;
    font-family:Arial,sans-serif;
}

header {
    padding:22px;
    border-bottom:1px solid #292932;
}

header h1 {
    margin:0;
    font-size:28px;
}

header span { color:#888; }

main {
    max-width:760px;
    margin:auto;
    padding:18px;
}

.status {
    background:#15151b;
    border:1px solid #292932;
    border-radius:10px;
    padding:14px;
    margin-bottom:16px;
}

.ok { color:#65df7c; }
.busy { color:#ffd166; }
.error { color:#ff6262; }

.tabs {
    display:flex;
    gap:8px;
    margin-bottom:16px;
}

.tabs button { flex:1; }

.card {
    background:#15151b;
    border:1px solid #292932;
    border-radius:10px;
    padding:18px;
    margin-bottom:16px;
}

h2 {
    margin:0 0 14px;
    font-size:19px;
}

button {
    background:#df3265;
    color:#fff;
    border:0;
    border-radius:7px;
    padding:12px 16px;
    font-weight:bold;
    cursor:pointer;
}

input, textarea, select {
    width:100%;
    padding:11px;
    margin:6px 0 14px;
    background:#0c0c10;
    color:#fff;
    border:1px solid #34343e;
    border-radius:6px;
}

textarea {
    min-height:110px;
    resize:vertical;
}

label {
    font-size:13px;
    color:#aaa;
}

.grid {
    display:grid;
    grid-template-columns:110px 1fr;
    gap:8px;
}

.key { color:#888; }

pre {
    padding:12px;
    background:#09090c;
    border-radius:6px;
    overflow:auto;
    white-space:pre-wrap;
    word-break:break-all;
}

.hidden { display:none; }

#log {
    max-height:160px;
    overflow:auto;
    font-family:monospace;
    font-size:12px;
    color:#aaa;
}

.logrow { margin-bottom:5px; }
</style>
</head>

<body>

<header>
    <h1>CherryRF</h1>
    <span>NFC Toolkit</span>
</header>

<main>

<div class="status">
    <span id="dot" class="ok">●</span>
    <span id="status">READER READY</span>
</div>

<div class="tabs">
    <button onclick="tab('read')">READ</button>
    <button onclick="tab('write')">WRITE</button>
    <button onclick="tab('copy')">COPY</button>
</div>

<section id="read">

    <div class="card">
        <h2>Tag Inspector</h2>
        <p>Present a tag to CherryRF.</p>
        <button onclick="readTag()">READ TAG</button>
    </div>

    <div id="tagCard" class="card hidden">
        <h2>Tag Information</h2>

        <div class="grid">
            <div class="key">UID</div>
            <div id="uid">-</div>

            <div class="key">Type</div>
            <div id="type">-</div>

            <div class="key">Capacity</div>
            <div id="capacity">-</div>

            <div class="key">Readable</div>
            <div id="readable">-</div>

            <div class="key">Writable</div>
            <div id="writable">-</div>

            <div class="key">NDEF</div>
            <div id="ndef">-</div>
        </div>
    </div>

    <div id="decodedCard" class="card hidden">
        <h2>Decoded Data</h2>
        <div id="decodedData"></div>
    </div>

    <div id="rawCard" class="card hidden">
        <h2>Raw Memory</h2>
        <pre id="raw"></pre>
    </div>

</section>

<section id="write" class="hidden">

    <div class="card">
        <h2>Write</h2>

        <label>Data type</label>

        <select id="writeType" onchange="writeTypeChanged()">
            <option value="text">Text</option>
            <option value="uri">URL</option>
        </select>

        <label id="valueLabel">Message</label>

        <textarea
            id="writeValue"
            placeholder="Hello from CherryRF!"></textarea>

        <button onclick="writeTag()">WRITE TAG</button>
    </div>

</section>

<section id="copy" class="hidden">

    <div class="card">
        <h2>Copy Tag</h2>

        <p id="copyStatus">Present the source tag.</p>

        <button id="readSource" onclick="copyRead()">READ SOURCE</button>

        <button
            id="writeTarget"
            class="hidden"
            onclick="copyWrite()">
            WRITE TARGET
        </button>
    </div>

    <div id="sourceCard" class="card hidden">
        <h2>Source Tag</h2>

        <div class="grid">
            <div class="key">UID</div>
            <div id="sourceUID">-</div>

            <div class="key">Type</div>
            <div id="sourceType">-</div>

            <div class="key">Data</div>
            <div id="sourceData">-</div>
        </div>
    </div>

</section>

<div class="card">
    <h2>Activity</h2>
    <div id="log"></div>
</div>

</main>

<script>
function tab(name)
{
    ["read","write","copy"].forEach(id =>
        document.getElementById(id).classList.add("hidden"));

    document.getElementById(name).classList.remove("hidden");
}

function setStatus(text, type="ok")
{
    document.getElementById("status").innerText = text;
    document.getElementById("dot").className = type;
}

function addLog(text)
{
    const row = document.createElement("div");
    row.className = "logrow";
    row.innerText = new Date().toLocaleTimeString() + "  " + text;

    document.getElementById("log").prepend(row);
}

async function request(url, options={})
{
    const response = await fetch(url, options);
    const data = await response.json();

    if (!response.ok)
        throw new Error(data.error || "REQUEST FAILED");

    return data;
}

async function readTag()
{
    setStatus("PRESENT TAG...", "busy");
    addLog("Waiting for tag");

    try
    {
        const d = await request("/api/read");

        document.getElementById("uid").innerText = d.uid;
        document.getElementById("type").innerText = d.type;
        document.getElementById("capacity").innerText = d.capacity + " bytes";
        document.getElementById("readable").innerText = d.readable ? "YES" : "NO";
        document.getElementById("writable").innerText = d.writable ? "YES" : "NO";
        document.getElementById("ndef").innerText = d.hasNDEF ? "YES" : "NO";

        document.getElementById("tagCard").classList.remove("hidden");

        const decoded = document.getElementById("decodedData");
        decoded.innerHTML = "";

        let hasDecodedData = false;

        if (d.records.length)
        {
            d.records.forEach((record, index) =>
            {
                const div = document.createElement("div");

                div.innerText =
                    "NDEF " +
                    record.type +
                    ": " +
                    record.value;

                decoded.appendChild(div);

                hasDecodedData = true;
            });
        }

        if (d.hasCherryRFData)
        {
            const div = document.createElement("div");

            div.innerText =
                "CherryRF " +
                d.cherryRFType +
                ": " +
                d.cherryRFValue;

            decoded.appendChild(div);

            hasDecodedData = true;
        }

        if (hasDecodedData)
            document.getElementById("decodedCard").classList.remove("hidden");
        else
            document.getElementById("decodedCard").classList.add("hidden");

        document.getElementById("raw").innerText = d.raw;
        document.getElementById("rawCard").classList.remove("hidden");

        setStatus("TAG READ");
        addLog("Read " + d.type + " " + d.uid);
    }
    catch(e)
    {
        setStatus(e.message, "error");
        addLog("Read failed: " + e.message);
    }
}

function writeTypeChanged()
{
    const type = document.getElementById("writeType").value;
    const input = document.getElementById("writeValue");

    if (type === "uri")
    {
        document.getElementById("valueLabel").innerText = "URL";
        input.placeholder = "https://example.com";
    }
    else
    {
        document.getElementById("valueLabel").innerText = "Message";
        input.placeholder = "Hello from CherryRF!";
    }
}

async function writeTag()
{
    const type = document.getElementById("writeType").value;
    const value = document.getElementById("writeValue").value.trim();

    if (!value)
    {
        setStatus("ENTER CONTENT", "error");
        return;
    }

    setStatus("PRESENT TAG...", "busy");
    addLog("Waiting to write " + type);

    const body =
        "type=" + encodeURIComponent(type) +
        "&value=" + encodeURIComponent(value);

    try
    {
        const d = await request("/api/write",
        {
            method:"POST",
            headers:{
                "Content-Type":"application/x-www-form-urlencoded"
            },
            body:body
        });

        setStatus("WRITE SUCCESSFUL");
        addLog("Write verified on " + d.uid);
    }
    catch(e)
    {
        setStatus(e.message, "error");
        addLog("Write failed: " + e.message);
    }
}

async function copyRead()
{
    setStatus("PRESENT SOURCE...", "busy");

    try
    {
        const d = await request("/api/copy/read", {method:"POST"});

        document.getElementById("sourceUID").innerText = d.uid;
        document.getElementById("sourceType").innerText = d.type;
        document.getElementById("sourceData").innerText = d.description;

        document.getElementById("sourceCard").classList.remove("hidden");
        document.getElementById("readSource").classList.add("hidden");
        document.getElementById("writeTarget").classList.remove("hidden");

        document.getElementById("copyStatus").innerText =
            "Remove source and present target tag.";

        setStatus("SOURCE CAPTURED");
        addLog("Source captured " + d.uid);
    }
    catch(e)
    {
        setStatus(e.message, "error");
        addLog("Source read failed: " + e.message);
    }
}

async function copyWrite()
{
    setStatus("PRESENT TARGET...", "busy");

    try
    {
        const d = await request("/api/copy/write", {method:"POST"});

        setStatus("COPY SUCCESSFUL");
        addLog("Copy verified on " + d.uid);

        document.getElementById("copyStatus").innerText =
            "Copy complete. Present another source tag.";

        document.getElementById("readSource").classList.remove("hidden");
        document.getElementById("writeTarget").classList.add("hidden");
        document.getElementById("sourceCard").classList.add("hidden");
    }
    catch(e)
    {
        setStatus(e.message, "error");
        addLog("Copy failed: " + e.message);
    }
}
</script>

</body>
</html>
)rawliteral";
}