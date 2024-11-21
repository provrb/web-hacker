# Web Hacker

# What it Does
Output all browser data from either Firefox or Chrome.

With this code, I learned how to decrypt Firefox and Chrome cookies, passwords and much more.

You are able to pull history, cookies, passwords, bookmarks and more.

I never managed to get credit cards from Firefox after spending many time debugging their strategies in x64dbg.

- One thing I do know is they use PK11_Decrypt, a function in nss.dll. You'd have to make a function pointer to decrypt the credit card #.
