
# **Move eCMD-pdbg Functionality to openpower-proc-control**

**Author:** Marri Devender Rao  
**Primary Assignee:** Marri Devender Rao  
**Contributors:** None  
**Date Created:** 10 November 2025  

---

## **1. Problem Description**

Currently, **ecmd-pdbg** is tightly coupled with the **eCMD** repository and cannot be built or maintained independently.  
The existing **ecmd-pdbg** recipe pulls the eCMD repository as a submodule and builds both the `ecmd` binaries and DLL.

Since both **eCMD** and **ecmd-pdbg** reside in the **open-power** layer, they cannot interact with **systemd**, preventing them from creating or committing **PELs (Platform Error Logs)**.

For next-generation **P12 systems**, which support multiple sleds requiring simultaneous power-on through a **systemd service**, it is essential to move the **ecmd-pdbg** functionality into **openpower-proc-control** (OpenBMC layer). This allows access to systemd and host firmware interfaces.

By relocating ecmd-pdbg, the new plugin will directly access:
- The **hostfw** repository for hardware access (scoms, cfams) and hardware procedures.

**Reference Links:**  
- [eCMD-pdbg Repository (Internal)](https://github.ibm.com/open-power/ecmd-pdbg)  
- [eCMD Repository (Open Source)](https://github.com/open-power/eCMD)  
- [openpower-proc-control Repository](https://github.com/ibm-openbmc/openpower-proc-control)

---

## **2. Objective**

The goal is to **decouple eCMD from ecmd-pdbg ** and build eCMD independently.

- eCMD will generate the header file `ecmdDllCapi.H`, defining the interface for client plugins.  
- Client layers (like openpower-proc-control) will implement this interface.  
- When an eCMD command (e.g., `ecmd istep`) is executed, eCMD dynamically loads the plugin defined by the environment variable:

  ```bash
  export ECMD_DLL_FILE=/usr/lib/libecmd-plugin.so
  ```

The implementation of `ecmdDllCapi.H` will now reside within **openpower-proc-control** as an **eCMD plugin**.

---

## **3. Background**

In the current setup, **ecmd-pdbg** depends on eCMD as a submodule, restricting flexibility and preventing systemd access.  
For **P12 multi-sled systems**, **openpower-proc-control** (running under systemd) manages power-on across sleds.  
Embedding eCMD integration here allows reuse of existing eCMD command-line functionality and direct access to host firmware APIs.

---

## **4. Requirements**

- Implement all functions defined in `ecmdDllCapi.H` within **openpower-proc-control** as an `ecmd-plugin`.  
- Ensure compatibility with eCMD binary interface and dynamic loading mechanism.  
- Support existing eCMD commands such as:
  - `getcfam`
  - `putscom`
  - `query`
  - `istep`

---

## **5. Proposed Design**

### **a. Build System and Recipe Changes**

- **ecmd bitbake recipe**
  - Build **eCMD** as an independent application.
  - Generate `ecmdDllCapi.H` for clients.

- **openpower-proc-control bitbake recipe**
  - Add dependency on `ecmd` recipe.
  - Include and build the **ecmd-plugin** implementing `ecmdDllCapi.H`.

---

### **b. eCMD Plugin Implementation**

- Implement all APIs defined in `ecmdDllCapi.H` inside a new module:
  ```
  openpower-proc-control/ecmd-plugin/
  ```
- eCMD dynamically loads this plugin using the `ECMD_DLL_FILE` environment variable.
- The plugin:
  - Provides hardware access via **hostfw** (scoms, cfams) and hardware procedures
  - Integrates seamlessly with systemd for coordinated sled bring-up
---

## **7. Alternatives Considered**

### **Option 1:**
Re-implement eCMD-like utilities directly inside openpower-proc-control.

**Drawbacks:**
- Significant development and maintenance overhead  
- Loss of mature eCMD CLI features  
- Incompatibility with existing tools (e.g., Cronus)

### **Option 2 (Chosen):**
Reuse eCMD as-is, and re-implement only the plugin interface (`ecmdDllCapi.H`) in openpower-proc-control.

---

## **8. Implementation Details**

- Add a new module:  
  ```
  openpower-proc-control/ecmd-plugin/
  ```
- Implement `ecmdDllCapi.H` API functions.  
- Use **hostfw** APIs for SCOM/CFAM access and hardware procedures  
- Validate plugin loading through:
  ```bash
  export ECMD_DLL_FILE=/usr/lib/libecmd-plugin.so
  ecmd istep 11.1
  ```
- Integrate with systemd services for multi-sled bring-up.

---

## **9. Testing Plan**

| **Test Case** | **Command** | **Expected Result** |
|----------------|-------------|---------------------|
| CFAM Read | `ecmd getcfam` | CFAM register read successful |
| SCOM Write | `ecmd putscom` | SCOM register write successful |
| Target Query | `ecmd query` | Target list displayed correctly |
| ISTEP Execution | `ecmd istep` | ISTEP executed via plugin successfully |

Successful completion of these validates end-to-end integration of **eCMD → ecmd-plugin → hostfw** under **systemd** orchestration.

---

## **10. Expected Outcome**

After migration:
- **ecmd-pdbg** functionality will reside in **openpower-proc-control**.  
- It will have full access to **systemd**, **hostfw**, and **multi-sled orchestration**.  
- The system will maintain eCMD’s command-line flexibility while enabling scalable next-generation bring-up workflows.
---

✅ **Final Benefit:**  
This refactoring ensures **better modularity, reusability, and integration** with OpenBMC services while preserving the rich eCMD feature set.
