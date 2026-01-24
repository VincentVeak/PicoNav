import discord
from discord.ext import commands
import re
import requests
from difflib import get_close_matches

# Bot setup
intents = discord.Intents.default()
intents.message_content = True
bot = commands.Bot(command_prefix='!', intents=intents)

# Store card database
cards_db = {}

def parse_cardlist_md(md_content):
    """Parse your CardList.md format"""
    cards = {}
    lines = md_content.split('\n')
    
    in_table = False
    for line in lines:
        # Start parsing after the header row
        if '| Icon | Name |' in line:
            in_table = True
            continue
        if in_table and line.startswith('|---'):
            continue
        
        if in_table and line.startswith('|'):
            parts = [p.strip() for p in line.split('|')]
            # parts[0] is empty, parts[1] is Icon, parts[2] is Name, etc.
            if len(parts) >= 7 and parts[2]:  # Has a name
                name = parts[2]
                if name and name != 'Name':  # Skip header
                    cards[name.lower()] = {
                        'name': name,
                        'description': parts[3],
                        'aspect': parts[4],
                        'series': parts[5],
                        'base_power': parts[6],
                        'tags': parts[7] if len(parts) > 7 else ''
                    }
    return cards

@bot.event
async def on_ready():
    global cards_db
    print(f'✅ {bot.user} is online!')
    
    # Fetch card data from GitHub
    github_url = 'https://raw.githubusercontent.com/YOUR_USERNAME/YOUR_REPO/main/docs/CardList.md'
    
    try:
        response = requests.get(github_url)
        cards_db = parse_cardlist_md(response.text)
        print(f'📦 Loaded {len(cards_db)} cards')
        await bot.change_presence(activity=discord.Game(name="[[cardname]] to search"))
    except Exception as e:
        print(f'❌ Error loading cards: {e}')

@bot.event
async def on_message(message):
    # Ignore bot's own messages
    if message.author == bot.user:
        return
    
    # Find all [[cardName]] patterns
    pattern = r'\[\[([^\]]+)\]\]'
    matches = re.findall(pattern, message.content)
    
    for card_query in matches:
        card_name_lower = card_query.lower().strip()
        
        # Exact match
        if card_name_lower in cards_db:
            card = cards_db[card_name_lower]
            await send_card_embed(message.channel, card)
        else:
            # Fuzzy match for typos
            all_card_names = list(cards_db.keys())
            close_matches = get_close_matches(card_name_lower, all_card_names, n=1, cutoff=0.6)
            
            if close_matches:
                card = cards_db[close_matches[0]]
                await send_card_embed(message.channel, card, suggested=True)
            else:
                await message.channel.send(f"❌ Card not found: `{card_query}`")
    
    # Allow commands to still work
    await bot.process_commands(message)

async def send_card_embed(channel, card, suggested=False):
    """Send formatted card info as Discord embed"""
    
    # Color based on aspect
    aspect_colors = {
        'Fire': 0xFF6B6B,
        'Water': 0x4A90E2,
        'Earth': 0x8B6F47,
        'Wind': 0x7DD3C0,
        'Neutral': 0x95A5A6
    }
    
    color = aspect_colors.get(card['aspect'], 0x000000)
    
    # Create embed
    title = card['name']
    if suggested:
        title = f"📝 Did you mean: {title}?"
    
    embed = discord.Embed(
        title=title,
        description=card['description'],
        color=color,
        url=f"https://www.piconav.com/docs/CardList.html"
    )
    
    # Add fields
    embed.add_field(name="⚡ Aspect", value=card['aspect'], inline=True)
    embed.add_field(name="📚 Series", value=card['series'], inline=True)
    embed.add_field(name="💪 Base Power", value=card['base_power'], inline=True)
    
    if card['tags']:
        embed.add_field(name="🏷️ Tags", value=card['tags'], inline=False)
    
    embed.set_footer(text="PicoNav Card Database • piconav.com")
    
    await channel.send(embed=embed)

# Helpful commands
@bot.command(name='refresh')
async def refresh_cards(ctx):
    """Reload card database from GitHub"""
    global cards_db
    
    github_url = 'https://raw.githubusercontent.com/YOUR_USERNAME/YOUR_REPO/main/docs/CardList.md'
    
    try:
        response = requests.get(github_url)
        cards_db = parse_cardlist_md(response.text)
        await ctx.send(f'✅ Reloaded {len(cards_db)} cards!')
    except Exception as e:
        await ctx.send(f'❌ Error: {e}')

@bot.command(name='cards')
async def list_cards(ctx, aspect=None):
    """List all cards or filter by aspect"""
    if aspect:
        filtered = [c['name'] for c in cards_db.values() if c['aspect'].lower() == aspect.lower()]
        if filtered:
            await ctx.send(f"**{aspect.title()} Cards:** {', '.join(filtered)}")
        else:
            await ctx.send(f"No cards found for aspect: {aspect}")
    else:
        await ctx.send(f"Total cards: {len(cards_db)}. Use `!cards [aspect]` to filter.")

# Run the bot
bot.run('YOUR_BOT_TOKEN_HERE')