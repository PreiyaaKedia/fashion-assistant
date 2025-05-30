@page "/"
@layout MainLayout
@using dotnetfashionassistant.Components.Layout;
@using Microsoft.Extensions.Configuration
@using Microsoft.Net.Http.Headers
@using System.Text
@using System.Text.Json
@using dotnetfashionassistant.Models
@rendermode InteractiveServer
@inject IHttpClientFactory HttpClientFactory
@inject IConfiguration configuration
@inject IJSRuntime JSRuntime
@inject dotnetfashionassistant.Services.AzureAIAgentService AzureAIAgentService
@inject dotnetfashionassistant.Services.AgentModeService AgentModeService
@inject dotnetfashionassistant.Services.CartUpdateService CartUpdateService

<div class="container">
    <div class="content">        
        <div class="d-flex justify-content-between align-items-center mb-4">
            <h1 class="page-title mb-0">Fashion Store Assistant</h1>
            <button class="btn btn-outline-primary" @onclick="StartNewConversation">
                <i class="bi bi-plus-circle me-2"></i>New Conversation
            </button>
        </div>
        
        <!-- AI Agent Chat Interface -->
        <div class="chat-container">
            <div class="chat-messages" id="chatMessages">
                @if (chatHistory.Count == 0)
                {
                    <div class="chat-message agent-message">
                        <div class="message-content">
                            <p>Hello! I'm your AI shopping assistant. How can I help you with your fashion needs today?</p>
                        </div>
                    </div>
                }
                else
                {
                    @foreach (var msg in chatHistory)
                    {                        <div class="chat-message @(msg.IsUser ? "user-message" : "agent-message")">
                            <div class="message-content">
                                @if (msg.IsUser)
                                {
                                    <p>@msg.Content</p>
                                }
                                else
                                {
                                    <p>@((MarkupString)(!string.IsNullOrEmpty(msg.FormattedContent) ? msg.FormattedContent : msg.Content))</p>
                                    
                                    @if (msg.ShowProductImages && msg.ProductIds.Any())
                                    {
                                        <div class="product-images-container">
                                            @foreach (var productId in msg.ProductIds)
                                            {
                                                <div class="product-image">
                                                    <img src="@Product.GetProductImageUrl(productId)" alt="Product Image" />
                                                    <div class="product-name">@products.FirstOrDefault(p => p.Id == productId)?.Name</div>
                                                </div>
                                            }
                                        </div>
                                    }
                                }
                            </div>
                        </div>
                    }
                }

                @if (isLoading)
                {
                    <div class="chat-message agent-message">
                        <div class="message-content typing-indicator">
                            <div class="typing-dot"></div>
                            <div class="typing-dot"></div>
                            <div class="typing-dot"></div>
                        </div>
                    </div>
                }
            </div>

            <div class="chat-input-container">
                <input type="text" 
                       class="chat-input" 
                       placeholder="Type your message here..." 
                       @bind="chatMessage" 
                       @bind:event="oninput" 
                       @onkeypress="HandleKeyPress" />
                <button class="chat-send-btn" @onclick="SendChatMessage" disabled="@(isLoading || string.IsNullOrWhiteSpace(chatMessage))">
                    <i class="bi bi-send-fill"></i>
                </button>
            </div>
        </div>
    </div>
</div>

<style>
    /* Center and top align the container */
    .container {
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: flex-start;
        min-height: 100vh;
        padding: 20px;
        text-align: center;
    }

    /* Narrow the heading */
    .page-title {
        font-size: 1.5em;
        width: auto;
        text-align: left;
    }

    /* Content styling */
    .content {
        width: 100%;
        max-width: 600px;
        text-align: left;
    }

    /* Chat interface styling */
    .chat-container {
        display: flex;
        flex-direction: column;
        height: 60vh;
        width: 100%;
        border: 1px solid #dee2e6;
        border-radius: 8px;
        overflow: hidden;
    }
    
    .chat-messages {
        flex-grow: 1;
        overflow-y: auto;
        padding: 15px;
        display: flex;
        flex-direction: column;
        gap: 10px;
        background-color: #f8f9fa;
    }
    
    .chat-message {
        display: flex;
        margin-bottom: 10px;
    }
    
    .user-message {
        justify-content: flex-end;
    }
    
    .agent-message {
        justify-content: flex-start;
    }
    
    .message-content {
        max-width: 70%;
        padding: 10px 15px;
        border-radius: 18px;
        word-wrap: break-word;
    }
    
    .user-message .message-content {
        background-color: #0d6efd;
        color: white;
        border-bottom-right-radius: 5px;
    }
    
    .agent-message .message-content {
        background-color: #e9ecef;
        color: #212529;
        border-bottom-left-radius: 5px;
    }
    
    .message-content p {
        margin: 0;
    }
    
    .chat-input-container {
        display: flex;
        padding: 15px;
        background-color: white;
        border-top: 1px solid #dee2e6;
    }
    
    .chat-input {
        flex-grow: 1;
        padding: 10px 15px;
        border: 1px solid #dee2e6;
        border-radius: 20px;
        outline: none;
        font-size: 1rem;
    }
    
    .chat-send-btn {
        width: 40px;
        height: 40px;
        border: none;
        background-color: #0d6efd;
        color: white;
        border-radius: 50%;
        margin-left: 10px;
        cursor: pointer;
        display: flex;
        align-items: center;
        justify-content: center;
    }
    
    .chat-send-btn:disabled {
        background-color: #6c757d;
        cursor: not-allowed;
    }
    
    /* Typing indicator */
    .typing-indicator {
        display: flex;
        align-items: center;
        padding: 10px 15px;
    }
    
    .typing-dot {
        width: 8px;
        height: 8px;
        background-color: #6c757d;
        border-radius: 50%;
        margin: 0 2px;
        animation: typing-animation 1.4s infinite both;
    }
    
    .typing-dot:nth-child(2) {
        animation-delay: 0.2s;
    }    .typing-dot:nth-child(3) {
        animation-delay: 0.4s;
    }

    /* Product images styling */
    .product-images-container {
        display: flex;
        flex-wrap: wrap;
        gap: 10px;
        margin-top: 10px;
        justify-content: center;
    }
    
    .product-image {
        width: 120px;
        text-align: center;
    }
    
    .product-image img {
        width: 100%;
        height: auto;
        border-radius: 5px;
        object-fit: cover;
    }
    
    .product-name {
        font-size: 0.8rem;
        margin-top: 5px;
        overflow: hidden;
        text-overflow: ellipsis;
        display: -webkit-box;
        -webkit-line-clamp: 2;
        -webkit-box-orient: vertical;
    }

    @@keyframes typing-animation {
        0% {
            opacity: 0.6;
            transform: scale(0.8);
        }
        50% {
            opacity: 1;
            transform: scale(1);
        }
        100% {
            opacity: 0.6;
            transform: scale(0.8);
        }
    }
</style>

@code {
    private bool isLoading;
    private string? chatMessage { get; set; }
    private List<ChatMessage> chatHistory = new List<ChatMessage>();
    private string? currentThreadId; // Store the agent thread ID for conversation continuity
    private bool isLoadingHistory = false;
    
    protected override async Task OnInitializedAsync()
    {
        // If we have a stored thread ID, use it
        if (!string.IsNullOrEmpty(AgentModeService.CurrentThreadId))
        {
            currentThreadId = AgentModeService.CurrentThreadId;
            
            // Add a welcome message while history loads
            if (chatHistory.Count == 0)
            {
                chatHistory.Add(new ChatMessage
                {
                    Content = "Loading conversation history...",
                    IsUser = false,
                    Timestamp = DateTime.Now
                });
            }
        }
        
        await base.OnInitializedAsync();
    }
    
    // Load chat history asynchronously to avoid blocking page navigation
    private async Task LoadChatHistoryAsync()
    {
        // If we have a thread ID and haven't started loading history yet
        if (!string.IsNullOrEmpty(currentThreadId) && !isLoadingHistory)
        {
            isLoadingHistory = true;
            
            try
            {
                var history = await AzureAIAgentService.GetThreadHistoryAsync(currentThreadId);
                
                // Update the chat history on the UI thread
                await InvokeAsync(() => {
                    chatHistory.Clear();
                    
                    // If no messages were loaded, add a welcome message
                    if (history.Count == 0)
                    {
                        chatHistory.Add(new ChatMessage
                        {
                            Content = "Hello! I'm your AI shopping assistant. How can I help you with your fashion needs today?",
                            IsUser = false,
                            Timestamp = DateTime.Now
                        });
                    }
                    else
                    {
                        // Add loaded history
                        chatHistory.AddRange(history);
                    }
                    
                    StateHasChanged();
                });
            }
            catch
            {
                // If there's an error loading history, show a friendly message
                await InvokeAsync(() => {
                    chatHistory.Clear();
                    chatHistory.Add(new ChatMessage
                    {
                        Content = "Hello! I'm your AI shopping assistant. How can I help you with your fashion needs today?",
                        IsUser = false,
                        Timestamp = DateTime.Now
                    });
                    StateHasChanged();
                });
            }
            finally
            {
                isLoadingHistory = false;
            }
        }    }
      // Start a new conversation by clearing the current thread ID and resetting the chat
    private void StartNewConversation()
    {
        // Clear the current thread ID
        currentThreadId = null;
        AgentModeService.CurrentThreadId = null;
        
        // Reset the chat history
        chatHistory.Clear();
        
        // Add a welcome message
        chatHistory.Add(new ChatMessage
        {
            Content = "Hello! I'm your AI shopping assistant. How can I help you with your fashion needs today?",
            IsUser = false,
            Timestamp = DateTime.Now
        });
        
        // Update the UI
        StateHasChanged();
        
        // Clear any cached data for the previous thread
        isLoadingHistory = false;
    }
      
    // Handle AI Agent chat functionality
    private async Task SendChatMessage()
    {
        if (string.IsNullOrWhiteSpace(chatMessage))
            return;
            
        // Add user message to chat history
        chatHistory.Add(new ChatMessage { 
            Content = chatMessage, 
            IsUser = true,
            Timestamp = DateTime.Now 
        });
        
        string userMessage = chatMessage;
        chatMessage = string.Empty; // Clear input field
        isLoading = true;
        StateHasChanged(); // Update UI to show user message
        
        try
        {
            // Create a thread if this is the first message
            if (string.IsNullOrEmpty(currentThreadId))
            {
                currentThreadId = await AzureAIAgentService.CreateThreadAsync();
                AgentModeService.CurrentThreadId = currentThreadId;
            }            // Send message to Azure AI Agent and get response
            string aiResponse = await AzureAIAgentService.SendMessageAsync(currentThreadId, userMessage);
            
            // Process the response and add to chat history
            ChatMessage chatMessage = ProcessAgentResponse(aiResponse);
            chatHistory.Add(chatMessage);
            
            // Check if the response mentions adding items to cart
            if (aiResponse.Contains("added to your cart") || 
                aiResponse.Contains("been added to your cart") || 
                aiResponse.Contains("Added to cart") ||
                aiResponse.Contains("item to your cart") ||
                aiResponse.Contains("items to your cart"))
            {
                // Update the cart counter if items were added
                await UpdateCartCounterAsync();
            }
        }
        catch (Exception ex)
        {
            // Handle any errors during AI communication
            chatHistory.Add(new ChatMessage { 
                Content = $"Sorry, I encountered an error: {ex.Message}. Please try again later.", 
                IsUser = false,
                Timestamp = DateTime.Now 
            });
        }
        finally
        {
            isLoading = false;
            StateHasChanged(); // Update UI with AI response
            
            // Scroll to bottom of chat
            await ScrollToBottom();
        }
    }
    
    private async Task HandleKeyPress(KeyboardEventArgs e)
    {
        if (e.Key == "Enter" && !string.IsNullOrWhiteSpace(chatMessage) && !isLoading)
        {
            await SendChatMessage();
        }
    }    private string FormatAgentResponse(string response)
    {
        // Format the response for better HTML display
        
        // Handle markdown-style bold text (convert **text** to <strong>text</strong>)
        response = System.Text.RegularExpressions.Regex.Replace(
            response, 
            @"\*\*([^*]+)\*\*", 
            "<strong>$1</strong>");
            
        // Handle line breaks and bullet points
        return response
            .Replace("\n\n", "<br><br>")
            .Replace("\n", "<br>")
            .Replace("•", "<br>•");
    }
    
    private ChatMessage ProcessAgentResponse(string aiResponse)
    {
        // Format the response for better HTML display
        string formattedResponse = FormatAgentResponse(aiResponse);
        
        // Create the message object
        var chatMessage = new ChatMessage { 
            Content = aiResponse,
            FormattedContent = formattedResponse,
            IsUser = false,
            Timestamp = DateTime.Now 
        };
        
        // Check if the response contains product information
        if (ContainsProductInformation(aiResponse))
        {
            chatMessage.ShowProductImages = true;
            
            // Add all product IDs to the message
            chatMessage.ProductIds.AddRange(products.Select(p => p.Id));
        }
        
        return chatMessage;
    }
    
    private bool ContainsProductInformation(string response)
    {
        // Case-insensitive check for both price and sizes
        string lowerResponse = response.ToLower();
        
        // Check if the response contains "price" and either "available sizes" or "sizes available"
        return lowerResponse.Contains("price") && 
               (lowerResponse.Contains("available sizes") || lowerResponse.Contains("sizes available"));
    }
    
    // Method to update the cart counter by fetching the current cart and updating the badge
    private async Task UpdateCartCounterAsync()
    {
        try
        {
            // Create a client for API calls
            var client = HttpClientFactory.CreateClient("LocalApi");
            
            // Get the current cart data
            var cartSummary = await client.GetFromJsonAsync<CartSummary>("api/Cart");
            
            if (cartSummary != null)
            {
                // Calculate total number of items in cart
                int totalItems = cartSummary.Items.Sum(item => item.Quantity);
                
                // Update the cart badge using JavaScript interop
                await JSRuntime.InvokeVoidAsync("updateCartBadge", totalItems);
                
                // Notify other components that cart has been updated
                CartUpdateService.NotifyCartUpdated();
            }
        }
        catch (Exception ex)
        {
            // Silently handle errors - we don't want to disturb the chat experience if cart update fails
            Console.WriteLine($"Error updating cart counter: {ex.Message}");
        }
    }
    
    private async Task ScrollToBottom()
    {
        try
        {
            // Use JSRuntime to scroll to the bottom of the chat
            await JSRuntime.InvokeVoidAsync("scrollToBottom", "chatMessages");
        }
        catch
        {
            // Silently fail if JS interop fails
        }
    }
      
    protected override async Task OnAfterRenderAsync(bool firstRender)
    {
        if (firstRender)
        {
            // Register JS function for scrolling
            await JSRuntime.InvokeVoidAsync("eval", @"
                window.scrollToBottom = function(elementId) {
                    var element = document.getElementById(elementId);
                    if (element) {
                        element.scrollTop = element.scrollHeight;
                    }
                }
            ");
            
            // Load chat history asynchronously after the first render for better performance
            await LoadChatHistoryAsync();
        }
        
        // Scroll to bottom whenever component rerenders
        await ScrollToBottom();
    }

    public static readonly List<Product> products = new List<Product>()
    {
        new Product { Id = 3, Name = "Navy Single-Breasted Slim Fit Formal Blazer", Description = "This navy single-breasted slim fit formal blazer is made from a blend of polyester and viscose. It features a notched lapel, a chest welt pocket, two flap pockets, a front button fastening, long sleeves, button cuffs, a double vent to the rear, and a full lining." },
        new Product { Id = 111, Name = "White & Navy Blue Slim Fit Printed Casual Shirt", Description = "White and navy blue printed casual shirt, has a spread collar, short sleeves, button placket, curved hem, one patch pocket" },
        new Product { Id = 116, Name = "Red Slim Fit Checked Casual Shirt", Description = "Red checked casual shirt, has a spread collar, long sleeves, button placket, curved hem, one patch pocket" },
        new Product { Id = 10, Name = "Navy Blue Washed Denim Jacket", Description = "Navy Blue washed denim jacket, has a spread collar, 4 pockets, button closure, long sleeves, straight hem, and unlined" }
    };    
    
    public class Product
    {
        public int Id { get; set; }
        public required string Name { get; set; }
        public required string Description { get; set; }

        public Product GetProduct(int Id) => products.Find(products => products.Id == Id) ?? throw new InvalidOperationException("Product not found");
        
        public static string GetProductImageUrl(int productId)
        {
            // Map product IDs to specific image files that match the product descriptions
            switch (productId)
            {
                case 3:
                    return "/images/products/navy-blazer.jpeg"; // Navy Single-Breasted Slim Fit Formal Blazer
                case 111:
                    return "/images/products/white-navy-shirt.jpeg"; // White & Navy Blue Slim Fit Printed Casual Shirt
                case 116:
                    return "/images/products/red-checked-shirt.jpeg"; // Red Slim Fit Checked Casual Shirt
                case 10:
                    return "/images/products/denim-jacket.jpeg"; // Navy Blue Washed Denim Jacket
                default:
                    // Fallback to a placeholder if the product ID is not recognized
                    return $"https://picsum.photos/seed/{productId}/400/228";
            }
        }
    }
}