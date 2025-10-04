// ============================================
// Clear inspector tab preference on page load
// Prevents opening to broken Layers tab
// ============================================
try {
  const inspectorTabKey = 'com.apple.WebInspector.selected-tab-index';
  const savedTab = localStorage.getItem(inspectorTabKey);
  if (savedTab !== null) {
    localStorage.removeItem(inspectorTabKey);
    console.log('[Inspector] Tab preference cleared (was:', savedTab, ') - will open to Elements tab');
  }
} catch (e) {
  // LocalStorage might not be available - not critical
  console.warn('[Inspector] Could not clear tab preference:', e);
}

let localhostReady = false;
const LOCALHOST_URL = 'http://localhost:8080/config';
const HEALTH_CHECK_INTERVAL = 2000; // 2 seconds

// Ultralight WebCore bug workaround: prevent rapid navigation
let lastNavigationTime = 0;
const MIN_NAVIGATION_DELAY = 500; // 500ms between navigations

function toggleSkyrimNetUIDiv(action) {
  const topMenu = document.getElementById("top-menu");
  const wrapper = document.getElementById("skyrimnet-ui");

  if (!topMenu || !wrapper) return;

  if (action === "show") {
    // Only show the menu bar, not the wrapper
    topMenu.classList.remove("hidden");
    topMenu.classList.add("visible");
  } else if (action === "hide") {
    // Hide both menu and wrapper
    topMenu.classList.remove("visible");
    topMenu.classList.add("hidden");
    wrapper.classList.remove("visible");
    wrapper.classList.add("hidden");
    // Clear button states when hiding
    clearAllButtonStates();
    // Tell iframe to pause its polling (3rd party SkyrimNet server optimization)
    sendIframeMessage("PAUSE");
  } else {
    // Toggle: only toggle the menu
    if (topMenu.classList.contains("visible")) {
      topMenu.classList.remove("visible");
      topMenu.classList.add("hidden");
      wrapper.classList.remove("visible");
      wrapper.classList.add("hidden");
      // Clear button states when toggling off
      clearAllButtonStates();
      // Tell iframe to pause its polling
      sendIframeMessage("PAUSE");
    } else {
      topMenu.classList.remove("hidden");
      topMenu.classList.add("visible");
    }
  }
}

function clearAllButtonStates() {
  document.getElementById('config-btn').classList.remove('active');
  document.getElementById('help-btn').classList.remove('active');
}

// Send message to iframe to pause/resume JavaScript polling
function sendIframeMessage(type) {
  const mainIframe = document.getElementById('skyrimnet-ui-iframe');
  if (mainIframe && mainIframe.contentWindow) {
    try {
      mainIframe.contentWindow.postMessage({ type: type }, '*');
      console.log(`[IFrame] Sent ${type} message to iframe`);
    } catch (e) {
      console.warn('[IFrame] Failed to send message:', e);
    }
  }
}

function switchToConfiguration() {
  const mainView = document.getElementById('main-view');
  const helpView = document.getElementById('help-view');
  const wrapper = document.getElementById('skyrimnet-ui');
  const configBtn = document.getElementById('config-btn');

  // If config button is already active, close the wrapper
  if (configBtn.classList.contains('active')) {
    wrapper.classList.remove('visible');
    wrapper.classList.add('hidden');
    clearAllButtonStates();
    // Pause iframe polling when closing
    sendIframeMessage('PAUSE');
    return;
  }

  // Show the wrapper if it's hidden
  if (wrapper.classList.contains('hidden')) {
    wrapper.classList.remove('hidden');
    wrapper.classList.add('visible');
  }

  // Switch to main view (regardless of current state)
  helpView.classList.remove('visible');
  helpView.classList.add('hidden');
  mainView.classList.remove('hidden');
  mainView.classList.add('visible');

  // Update button states - only config should be active
  clearAllButtonStates();
  configBtn.classList.add('active');

  // Resume iframe polling when showing configuration
  sendIframeMessage('RESUME');
}

function switchToHelp() {
  const mainView = document.getElementById('main-view');
  const helpView = document.getElementById('help-view');
  const wrapper = document.getElementById('skyrimnet-ui');
  const helpBtn = document.getElementById('help-btn');

  // If help button is already active, close the wrapper
  if (helpBtn.classList.contains('active')) {
    wrapper.classList.remove('visible');
    wrapper.classList.add('hidden');
    clearAllButtonStates();
    // Pause iframe polling when closing
    sendIframeMessage('PAUSE');
    return;
  }

  // Show the wrapper if it's hidden
  if (wrapper.classList.contains('hidden')) {
    wrapper.classList.remove('hidden');
    wrapper.classList.add('visible');
  }

  // Switch to help view (regardless of current state)
  mainView.classList.remove('visible');
  mainView.classList.add('hidden');
  helpView.classList.remove('hidden');
  helpView.classList.add('visible');

  // Update button states - only help should be active
  clearAllButtonStates();
  helpBtn.classList.add('active');

  // Pause main view iframe when switching to help
  // (Help view has its own iframe with external docs)
  sendIframeMessage('PAUSE');
}

function updateGameMasterStatus(enabled) {
  console.log('[GameMaster] updateGameMasterStatus called with:', enabled, 'type:', typeof enabled);

  const statusElement = document.getElementById('gamemaster-status');

  // Convert string to boolean (InteropCall sends "true" or "false" as strings)
  const isEnabled = (enabled === true || enabled === "true");

  console.log('[GameMaster] Converted to boolean:', isEnabled);

  if (isEnabled) {
    statusElement.innerHTML = '<span class="status-ready">✅ Ready</span>';
    console.log('[GameMaster] UI updated to: Ready');
  } else {
    statusElement.innerHTML = '<span class="status-disabled">🔴 Agent Disabled</span>';
    console.log('[GameMaster] UI updated to: Agent Disabled');
  }
}

function onGameMasterClick() {
  console.log('GameMaster button clicked');

  // Note: GameMaster button doesn't show the wrapper or change views
  // It only toggles the GameMaster agent state

  if (window.onGameMasterToggle) {
    window.onGameMasterToggle();
  }
}

function toggleHelpPanel() {
  const mainView = document.getElementById('main-view');
  const helpView = document.getElementById('help-view');
  const configBtn = document.getElementById('config-btn');
  const helpBtn = document.getElementById('help-btn');

  // Toggle visibility and update button states accordingly
  if (mainView.classList.contains('visible')) {
    mainView.classList.remove('visible');
    mainView.classList.add('hidden');
    helpView.classList.remove('hidden');
    helpView.classList.add('visible');
    // Update button states for help view
    clearAllButtonStates();
    helpBtn.classList.add('active');
  } else {
    helpView.classList.remove('visible');
    helpView.classList.add('hidden');
    mainView.classList.remove('hidden');
    mainView.classList.add('visible');
    // Update button states for config view
    clearAllButtonStates();
    configBtn.classList.add('active');
  }
}

function showIframeError(iframe, message) {
  const errorDoc = `
    <!DOCTYPE html>
    <html>
    <head>
      <style>
        body {
          margin: 0;
          padding: 20px;
          font-family: sans-serif;
          background: #1a1a1a;
          color: #22c55e;
          display: flex;
          align-items: center;
          justify-content: center;
          height: 100vh;
        }
        .error-container {
          text-align: center;
          max-width: 500px;
        }
        h2 { color: #ff5555; margin-bottom: 16px; }
        p { margin: 8px 0; line-height: 1.6; }
        .retry-btn {
          margin-top: 20px;
          padding: 10px 20px;
          background: #22c55e;
          color: #000;
          border: none;
          border-radius: 4px;
          cursor: pointer;
          font-weight: bold;
        }
        .retry-btn:hover { background: #16a34a; }
      </style>
    </head>
    <body>
      <div class="error-container">
        <h2>⚠ Unable to Load Content</h2>
        <p>${message}</p>
        <p><small>Check that the server is running and accessible.</small></p>
        <button class="retry-btn" onclick="window.location.reload()">Retry</button>
      </div>
    </body>
    </html>
  `;

  try {
    const doc = iframe.contentDocument || iframe.contentWindow.document;
    doc.open();
    doc.write(errorDoc);
    doc.close();
  } catch (e) {
    console.error('Could not inject error page:', e);
  }
}

function reloadIframe() {
  const mainView = document.getElementById('main-view');
  const helpView = document.getElementById('help-view');
  let iframe;

  try {
    if (mainView.classList.contains('visible')) {
      iframe = mainView.querySelector('iframe');
      if (iframe) {
        const currentSrc = iframe.src;
        // Force reload by temporarily clearing src
        iframe.src = 'about:blank';
        setTimeout(() => {
          iframe.src = currentSrc;
        }, 50);
      }
    } else {
      iframe = helpView.querySelector('iframe');
      if (iframe) {
        const currentSrc = iframe.src;
        iframe.src = 'about:blank';
        setTimeout(() => {
          iframe.src = currentSrc;
        }, 50);
      }
    }
  } catch (e) {
    console.error('Reload failed:', e);
    if (iframe) {
      showIframeError(iframe, 'Reload failed due to security restrictions or network error.');
    }
  }
}

async function checkLocalhostHealth() {
  try {
    const response = await fetch(LOCALHOST_URL, {
      method: 'HEAD',
      mode: 'no-cors', // Allows check even without CORS headers
      cache: 'no-cache'
    });
    // no-cors returns opaque response, so we assume success if no error thrown
    return true;
  } catch (e) {
    return false;
  }
}

async function pollLocalhost() {
  if (localhostReady) return;

  const isHealthy = await checkLocalhostHealth();
  if (isHealthy) {
    localhostReady = true;
    console.log('Localhost server is ready');

    // Update iframe src now that server is available
    const mainIframe = document.getElementById('skyrimnet-ui-iframe');
    if (mainIframe && mainIframe.src === 'about:blank') {
      mainIframe.src = LOCALHOST_URL;
    }
  } else {
    console.log('Waiting for localhost server...');
    setTimeout(pollLocalhost, HEALTH_CHECK_INTERVAL);
  }
}

function setupIframeErrorHandlers() {
  const mainIframe = document.getElementById('skyrimnet-ui-iframe');
  const helpIframe = document.querySelector('#help-view iframe');

  if (mainIframe) {
    mainIframe.addEventListener('error', function() {
      console.error('Main iframe failed to load');
      showIframeError(mainIframe, `Unable to connect to ${LOCALHOST_URL}. Server may not be running yet.`);
    });

    mainIframe.addEventListener('load', function() {
      // Check if iframe loaded an error page (about:blank or empty)
      try {
        const doc = mainIframe.contentDocument;
        if (doc && doc.location.href === 'about:blank') {
          console.warn('Main iframe loaded blank page');
        } else {
          console.log('Main iframe loaded successfully:', doc?.location?.href || 'cross-origin');

          // Record navigation time for rate limiting (Ultralight WebCore crash workaround)
          lastNavigationTime = Date.now();

          // Monitor iframe for navigation attempts that might break out
          try {
            if (doc && doc.location.href.startsWith('http://localhost')) {
              // Set base target to keep all navigation in iframe
              let base = doc.querySelector('base');
              if (!base) {
                base = doc.createElement('base');
                base.target = '_self';
                (doc.head || doc.documentElement).appendChild(base);
              }

              // Intercept link clicks to add delay (Ultralight WebCore crash workaround)
              doc.addEventListener('click', function(e) {
                const link = e.target.closest('a');
                if (link && link.href) {
                  const timeSinceLastNav = Date.now() - lastNavigationTime;
                  if (timeSinceLastNav < MIN_NAVIGATION_DELAY) {
                    e.preventDefault();
                    const remainingDelay = MIN_NAVIGATION_DELAY - timeSinceLastNav;
                    console.log(`[Navigation] Delaying ${remainingDelay}ms to prevent WebCore crash`);
                    setTimeout(() => {
                      // Navigate within iframe context
                      doc.location.href = link.href;
                    }, remainingDelay);
                  }
                }
              }, true);
            }
          } catch (e) {
            // Cross-origin, can't inject base tag
            console.warn('Cannot inject base tag (cross-origin)');
          }
        }
      } catch (e) {
        // Cross-origin, can't check - assume success
        console.log('Main iframe loaded (cross-origin)');
      }
    });
  }

  if (helpIframe) {
    helpIframe.addEventListener('error', function() {
      console.error('Help iframe failed to load');
      showIframeError(helpIframe, 'Unable to load documentation. This site may block iframe embedding.');
    });

    helpIframe.addEventListener('load', function() {
      console.log('Help iframe loaded');
    });
  }
}

// Protect parent window from being navigated by iframe
window.addEventListener('beforeunload', function(e) {
  // Check if navigation is from parent or from iframe breaking out
  const currentUrl = window.location.href;
  if (!currentUrl.includes('PrismaUI-SkyrimNet-UI')) {
    console.warn('Prevented navigation away from PrismaUI window');
    e.preventDefault();
    e.returnValue = '';
    return '';
  }
});

function closeUI() {
  // Only close the wrapper, keep the menu visible
  const wrapper = document.getElementById('skyrimnet-ui');
  wrapper.classList.remove('visible');
  wrapper.classList.add('hidden');
  // Clear button states when closing wrapper
  clearAllButtonStates();
}

// Dragging and Resizing logic
window.addEventListener('DOMContentLoaded', function() {
  const wrapper = document.getElementById('skyrimnet-ui');
  const navbar = document.querySelector('.navbar');
  const handle = document.querySelector('.resize-handle');

  // Set up iframe error handlers
  setupIframeErrorHandlers();

  // Start polling for localhost availability
  pollLocalhost();

  // Dragging functionality
  let isDragging = false;
  let dragOffsetX = 0, dragOffsetY = 0;

  navbar.addEventListener('mousedown', function(e) {
    if (e.target.classList.contains('navbar-btn')) return;
    isDragging = true;

    // Calculate current position accounting for transform
    const rect = wrapper.getBoundingClientRect();
    dragOffsetX = e.clientX - rect.left;
    dragOffsetY = e.clientY - rect.top;

    // Remove centering transform on first drag
    if (!wrapper.classList.contains('dragged')) {
      wrapper.classList.add('dragged');
      wrapper.style.left = rect.left + 'px';
      wrapper.style.top = rect.top + 'px';
    }

    document.body.style.cursor = 'move';
    e.preventDefault();
  });

  // Resizing functionality
  let isResizing = false;
  let lastX = 0, lastY = 0;
  let startWidth = 0, startHeight = 0;

  handle.addEventListener('mousedown', function(e) {
    e.preventDefault();
    e.stopPropagation();
    isResizing = true;
    lastX = e.clientX;
    lastY = e.clientY;
    startWidth = wrapper.offsetWidth;
    startHeight = wrapper.offsetHeight;
    document.body.style.cursor = 'se-resize';
  });

  document.addEventListener('mousemove', function(e) {
    if (isDragging) {
      let newLeft = e.clientX - dragOffsetX;
      let newTop = e.clientY - dragOffsetY;
      // Keep within viewport bounds
      newLeft = Math.max(0, Math.min(newLeft, window.innerWidth - wrapper.offsetWidth));
      newTop = Math.max(0, Math.min(newTop, window.innerHeight - wrapper.offsetHeight));
      wrapper.style.left = newLeft + 'px';
      wrapper.style.top = newTop + 'px';
    }

    if (isResizing) {
      const dx = e.clientX - lastX;
      const dy = e.clientY - lastY;
      let newWidth = Math.max(400, startWidth + dx);
      let newHeight = Math.max(300, startHeight + dy);
      wrapper.style.width = newWidth + 'px';
      wrapper.style.height = newHeight + 'px';
    }
  });

  document.addEventListener('mouseup', function(e) {
    if (isDragging) {
      isDragging = false;
      document.body.style.cursor = '';
    }
    if (isResizing) {
      isResizing = false;
      document.body.style.cursor = '';
    }
  });
});