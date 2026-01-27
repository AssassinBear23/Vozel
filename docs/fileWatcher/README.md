# FileWatcher Documentation Guide

This directory contains comprehensive documentation for the FileWatcher system used in the FinalEngine editor. All diagrams use Mermaid syntax and can be viewed in any Markdown viewer that supports Mermaid (GitHub, VS Code with plugins, etc.).

---

## Quick Start (ELI5 Explanations)

Think of the FileWatcher like having two friends: **Bobby (Worker Thread)** and **Sally (Main Thread/UI)**.

- **Bobby** watches your toy box (assets folder) 24/7 and writes notes when toys change
- **Sally** runs the game editor and checks Bobby's notes 60 times per second
- They use a **mailbox** (event queue) to share notes safely
- A **lock** on the mailbox makes sure only one person uses it at a time

---

## Core Architecture Files

### fileWatcherClassDiagram.md
**What it shows:** The complete class structure with all members and relationships

1. **Top layer (Sally)** - The UI that draws on screen
2. **Middle layer (Coordinator)** - The manager that connects Sally and Bobby
3. **Bottom layer (Bobby)** - The helper that watches files

**Key classes:**
- `ProjectPanel` - Sally (draws the file list)
- `FileWatcher` - The manager
- `FileWatcherWorker` - Bobby (background watcher)
- `FileCache` - The memory/notebook
- `FileTreeNode` - One file or folder in the tree

---

### Relations_HighLevel_ClassDiagram.md
**What it shows:** Simplified version focusing on ownership and data flow

This is the "big picture" version. Instead of showing every detail, it shows:
- Who owns what (ProjectPanel owns FileWatcher owns FileWatcherWorker)
- How data flows (FileWatcher → Queue → Editor/UI)
- What each layer is responsible for

**Use this when:** You want to understand the overall structure without getting lost in details.

---

## Flow Diagrams (How Things Work)

### fileWatcherEventGraph.md
**What it shows:** The complete lifecycle from editor startup to shutdown

**Like a 5-year-old would understand:**

This shows the whole day in the life of the FileWatcher:

1. **Morning** - Editor starts, Bobby is hired and starts watching
2. **During the day** - Bobby detects changes and writes notes
3. **Every moment** - Sally checks mailbox 60 times per second
4. **Evening** - Editor closes, Bobby is thanked and goes home

**Key parts:**
- Initial setup (blue nodes) - Getting ready
- Worker thread loop (purple nodes) - Bobby's job
- Main thread loop (green nodes) - Sally's job
- Shutdown sequence (red nodes) - Going home safely

---

### ThreadCommunication.md
**What it shows:** Sequence diagram of one complete event cycle (16ms)

**Like a 5-year-old would understand:**

This shows exactly what happens when you save a file:

1. You save `texture.png` in Photoshop
2. Windows tells Bobby
3. Bobby writes a note and puts it in the mailbox
4. 16 milliseconds later (next frame)
5. Sally checks the mailbox
6. Sally reads the note and logs it
7. Sally draws the file tree on screen

**Timeline:** Shows the exact order things happen in.

---

### ProjectPanelFlow.md
**What it shows:** What happens inside `ProjectPanel::draw()` every frame

**Like a 5-year-old would understand:**

This shows Sally's job every 16 milliseconds:

1. **Check mailbox** - Are there new notes from Bobby?
2. **Read notes** - Log them to console, update counter
3. **Draw UI** - Always draw (ImGui does this automatically)
   - Draw toolbar with refresh button
   - Draw file tree with folders and files
   - Handle clicks (when you select a file)

**Important:** Sally draws EVERY frame, even if there are no new notes. That's the "ImGui way"!

---

### EventParsing.md
**What it shows:** How Bobby reads Windows' weird messages and turns them into simple notes

**Like a 5-year-old would understand:**

Windows speaks a complicated language with weird symbols. Bobby's job is to translate:
- Windows says: "FILE_ACTION_MODIFIED, 𝕥𝕖𝕩𝕥𝕦𝕣𝕖.𝕡𝕟𝕘"
- Bobby translates to: "FileEvent{Modified, 'assets/textures/texture.png'}"

**Steps:**

1. Windows gives Bobby a big piece of paper (buffer)
2. Bobby reads the weird symbols (wide strings)
3. Bobby converts to normal English (UTF-8)
4. Bobby creates a simple note Sally can understand
5. Bobby puts note in mailbox

---

## Thread Synchronization Files

### MutexOperations.md
**What it shows:** How the mailbox lock prevents chaos

**Like a 5-year-old would understand:**

Imagine Bobby and Sally both try to use the mailbox at the same time:
- Bobby wants to put a note IN
- Sally wants to take a note OUT

If they both grab it at once, notes fall everywhere!

**The lock dance:**

1. Bobby arrives with note
2. Tries to open lock
3. Is Sally using it?
   - YES: Bobby waits (sleeps)
   - NO: Bobby opens lock
4. Bobby puts note inside
5. Lock automatically closes when Bobby is done
6. Bobby walks away

Same dance for Sally when she wants to read notes.

---

### ShutdownOfThreads.md
**What it shows:** How to safely stop Bobby when closing the editor

**Like a 5-year-old would understand:**

When it's bedtime, you can't just turn off the lights while Bobby is still working! You need to:

1. **Tell Bobby** - "Hey, it's bedtime!" (set shutdown flag)
2. **Wake Bobby up** - Ring a bell if he's sleeping (SetEvent)
3. **Wait for Bobby** - Let him finish and clean up (thread.join)
4. **Lock the door** - Only after Bobby has left (close handles)

**Important:** If you don't wait for Bobby, bad things happen (crashes, memory leaks)!

---

# Optimization Techniques

### AtomicShutdownPattern.md
**What it shows:** The special "stop sign" both Bobby and Sally can see

**Like a 5-year-old would understand:**

The shutdown flag is like a magic stop sign:
- When Sally sets it to "STOP", Bobby can see it immediately
- No confusion, no delay, no mix-ups
- Both can look at it safely at the same time

**Why special?** Normal variables can cause "race conditions" where Bobby and Sally see different things. Atomic variables prevent this magic.

---

## Best Practices Files

### EventCoalescing.md
**What it shows:** Combining many similar events into one

**Like a 5-year-old would understand:**

Your friend knocks on your door 10 times super fast. Instead of answering 10 times, you wait a few seconds and answer once.

**Example:**

Without coalescing: File modified 10 times in 100ms = 10 events, 10 reloads (slow!)

With coalescing: File modified 10 times in 100ms = 1 event after 250ms, 1 reload (fast!)

---

### EventFiltering.md
**What it shows:** Ignoring files we don't care about

**Like a 5-year-old would understand:**

When looking for your favorite toy, you don't pick up every single thing in the toy box - you ignore broken crayons and empty boxes.

**What we ignore:**
- `.tmp` files (temporary junk)
- `.git` files (internal Git stuff)
- `.vs` files (Visual Studio cache)

**What we care about:**
- `.png`, `.jpg` (textures)
- `.fbx`, `.obj` (models)
- `.vert`, `.frag` (shaders)

---

### LockContention.md
**What it shows:** How to use the mailbox lock efficiently

**Like a 5-year-old would understand:**

Instead of Bobby locking the mailbox 5 times to put 5 notes, he carries all 5 notes at once and locks the mailbox just once.

**Bad way (5 locks):**

Lock → Put note 1 → Unlock

Lock → Put note 2 → Unlock

Lock → Put note 3 → Unlock  (Sally might be waiting!)

**Good way (1 lock):**

Carry 5 notes

Lock → Put all 5 notes → Unlock

---

### LockContention_Effect.md
**What it shows:** What happens when Sally has to wait for the lock

**Like a 5-year-old would understand:**

Sally needs to draw 60 pictures per second (60 FPS). Each picture should take 16.67ms.

If Bobby holds the lock too long, Sally has to wait, and she only draws 59 pictures that second. The game looks stuttery!

**Timeline:**
- Frame normally takes: 16.67ms
- Bobby holds lock: 0.1ms
- Sally waits: 0.1ms
- Frame now takes: 16.8ms X (dropped to 59 FPS)

---

### NonBlockingMain.md
**What it shows:** How Sally processes events without freezing

**Like a 5-year-old would understand:**

If Sally gets 100 notes at once, she doesn't read all 100 right away (that takes too long!). Instead:
- **Few notes (< 10):** Read them all immediately
- **Many notes (100+):** Read 10 this frame, 10 next frame, etc.

This keeps the editor smooth (60 FPS) even when there are tons of changes.

---

# Key Design Decisions

### What We Chose

**1. Direct Polling (No Observer Pattern)**
- Sally directly asks Bobby for events
- Simpler than complex listener interfaces

**2. No Dirty Flags (ImGui Immediate Mode)**
- Sally draws EVERY frame (16ms)
- No need to mark UI as "dirty"

**3. Windows-Specific (For Now)**
- Uses `ReadDirectoryChangesW` API
- Very efficient (Bobby sleeps until changes happen)

**4. Mutex-Protected Queue**
- Simple `std::mutex` + `std::queue`
- Lock-free not needed (events are rare)

---

## Common Questions

**Q: Why does Sally draw every frame even without events?**

A: That's how ImGui works! It's "immediate mode" - you describe the UI every frame, and ImGui figures out what changed. It's actually very efficient.

**Q: Why Windows-only for now?**

A: Keeps the implementation simple, and the engine is currently programmed to only run on windows specifically.

**Q: Can Bobby crash the editor?**

A: No! Bobby and Sally communicate only through the mutex-protected queue. If Bobby crashes, Sally keeps running (though file watching stops).

**Q: Why "Bobby" and "Sally"?**

A: Makes it easier to talk about threads! Bobby = Worker Thread, Sally = Main Thread/UI.

---

## Metadata
**Document Version:** 1.0  
**Last Updated:** 27 January 2026