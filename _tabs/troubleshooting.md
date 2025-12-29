   ---
   layout: page
   icon: fas fa-book
   order: 2
   ---

# Troubleshooting

Common issues and solutions.

## Device won't power on

1. Check USB cable connection
2. Try a different USB port
3. Verify power supply

## Connection issues

...

## Still need help?

- [Open an issue on GitHub](https://github.com/yourusername/piconav/issues)
- [Email support](mailto:support@veak.design)
```

---

## **Step 4: Upload Files to GitHub**

**Option A: Via GitHub Website (Easiest)**
1. Go to your repository
2. Click **"Add file"** → **"Create new file"**
3. Name it `_config.yml`
4. Paste the content
5. Click **"Commit new file"**
6. Repeat for each file

**Option B: GitHub Desktop (Recommended)**
1. Download [GitHub Desktop](https://desktop.github.com/)
2. Clone your repository
3. Add all the files locally
4. Commit and push

---

## **Step 5: Enable GitHub Pages**

1. Go to your repository on GitHub
2. Click **"Settings"** tab
3. Scroll to **"Pages"** in the left sidebar
4. Under **"Source"**, select:
   - Branch: `main`
   - Folder: `/ (root)`
5. Click **"Save"**
6. Wait 1-2 minutes for the site to build

Your site will initially be at: `https://yourusername.github.io/piconav-docs/`

---

## **Step 6: Connect piconav.com Custom Domain**

### **In GitHub:**
1. Still in **Settings → Pages**
2. Under **"Custom domain"**, enter: `piconav.com`
3. Click **"Save"**
4. Check ✅ **"Enforce HTTPS"** (after DNS propagates)

### **In Your Domain DNS Settings:**

You need to update the DNS for piconav.com. Since it's connected to Shopify right now, you'll need to:

**Option 1: Remove from Shopify, point to GitHub**
1. Go to Shopify → Settings → Domains
2. Remove or disconnect piconav.com
3. Go to your domain registrar (where you bought the domain)
4. Add these DNS records:
```
Type: A
Name: @
Value: 185.199.108.153

Type: A
Name: @
Value: 185.199.109.153

Type: A
Name: @
Value: 185.199.110.153

Type: A
Name: @
Value: 185.199.111.153

Type: CNAME
Name: www
Value: yourusername.github.io
